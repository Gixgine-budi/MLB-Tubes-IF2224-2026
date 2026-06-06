#include <algorithm>

#include "ast/expr_nodes.hpp"
#include "ast/type_nodes.hpp"
#include "semantic/semantic_analyzer.hpp"
#include "semantic/symtable_entries.hpp"

namespace semantic {

namespace {

int variableStackSize(const SymbolTable& sym_table, int type_tab_idx) {
  const auto& type_entry = sym_table.getTabEntry(type_tab_idx);
  if (type_entry.type == static_cast<int>(BuiltinType::Array) &&
      type_entry.ref > 0) {
    const auto& atab_entry = sym_table.getAtabEntry(type_entry.ref);
    return std::max(1, atab_entry.size) * std::max(1, atab_entry.elsz);
  }
  return 1;
}

}  // namespace

int SemanticAnalyzer::makeAnonymousType(int raw_type, int ref) {
  const std::string name =
      "$anonym_type" + std::to_string(anon_type_counter_++);
  return sym_table.enterTab(name, ObjClass::Type, raw_type, ref);
}

int SemanticAnalyzer::constIntValue(const ast::AstNode* node,
                                    int fallback) const {
  if (node == nullptr) return fallback;

  if (auto value = constantValue(*node)) {
    return value->value;
  }

  return fallback;
}

int SemanticAnalyzer::resolveSimpleTypeName(const std::string& name,
                                            const lexer::Token* token) {
  if (auto entry = sym_table.lookup(name)) {
    if (entry->obj == ObjClass::Type) {
      if (entry->ref > 0) {
        return entry->idx;
      }
      if (entry->type >= RESERVED) {
        return entry->type;
      }
      return entry->idx;
    }
  }
  reportError("unknown type '" + name + "'", token);
  return 0;
}

int SemanticAnalyzer::resolveTypeSpec(ast::TypeSpecNode& spec) {
  spec.accept(*this);
  return spec.expression_type;
}

void SemanticAnalyzer::visit(ast::SimpleTypeSpecNode& node) {
  node.expression_type = resolveSimpleTypeName(node.name.lexeme, &node.name);
}

void SemanticAnalyzer::visit(ast::SubrangeTypeSpecNode& node) {
  if (node.low != nullptr) node.low->accept(*this);
  if (node.high != nullptr) node.high->accept(*this);

  int base_type = 0;
  int low_value = 0;
  int high_value = 0;

  if (node.low != nullptr && node.high != nullptr) {
    const int low_type = node.low->expression_type;
    const int high_type = node.high->expression_type;
    const int low_base = scalarBaseType(low_type);
    const int high_base = scalarBaseType(high_type);
    const auto low_const = constantValue(*node.low);
    const auto high_const = constantValue(*node.high);

    if (!isOrdinalLike(low_type)) {
      reportError("subrange lower bound must be an ordinal type",
                  sourceToken(*node.low));
    } else if (low_base != high_base) {
      reportError("subrange upper bound must match lower bound type",
                  sourceToken(*node.high));
    } else if (!low_const || !high_const) {
      reportError("subrange bounds must be constant values",
                  sourceToken(!low_const ? *node.low : *node.high));
    } else {
      base_type = low_base;
      low_value = low_const->value;
      high_value = high_const->value;
      if (low_value > high_value) {
        reportError("subrange lower bound must not exceed upper bound",
                    sourceToken(*node.low));
      }
    }
  }

  AtabEntry bounds{};
  bounds.xtyp = base_type;
  bounds.etyp = base_type;
  bounds.low = low_value;
  bounds.high = high_value;
  bounds.elsz = 1;
  bounds.size = high_value >= low_value ? (high_value - low_value + 1) : 1;

  const int bounds_idx = sym_table.enterTab(bounds);
  node.expression_type =
      makeAnonymousType(static_cast<int>(BuiltinType::Subrange), bounds_idx);
}

void SemanticAnalyzer::visit(ast::ArrayTypeSpecNode& node) {
  const int index_type =
      (node.index_type != nullptr) ? resolveTypeSpec(*node.index_type) : 0;
  const int element_type =
      (node.element_type != nullptr) ? resolveTypeSpec(*node.element_type) : 0;

  int element_ref = 0;
  if (element_type >= RESERVED) {
    element_ref = sym_table.getTabEntry(element_type).ref;
  }

  AtabEntry atab{};
  atab.xtyp = index_type;
  atab.etyp = element_type;
  atab.eref = element_ref;
  atab.low = 0;
  atab.high = 0;
  atab.elsz = 1;
  atab.size = 1;

  if (const auto* subrange = dynamic_cast<const ast::SubrangeTypeSpecNode*>(
          node.index_type.get())) {
    atab.low = constIntValue(subrange->low.get(), 0);
    atab.high = constIntValue(subrange->high.get(), 0);
    atab.size = atab.high >= atab.low ? (atab.high - atab.low + 1) : 1;
  }

  const int atab_idx = sym_table.enterTab(atab);
  node.expression_type =
      makeAnonymousType(static_cast<int>(BuiltinType::Array), atab_idx);
}

void SemanticAnalyzer::visit(ast::RecordTypeSpecNode& node) {
  const int record_block = sym_table.pushBlock();

  for (auto& field : node.fields) {
    const int field_type =
        (field.second != nullptr) ? resolveTypeSpec(*field.second) : 0;

    for (const auto& id : field.first) {
      if (sym_table.lookupCurrentScope(id.lexeme)) {
        reportError("record field '" + id.lexeme + "' already declared", &id);
        continue;
      }
      const int stack_size = variableStackSize(sym_table, field_type);
      sym_table.enterTab(id.lexeme, ObjClass::Variable, field_type, 0, 1,
                         stack_size);
    }
  }

  sym_table.popBlock();
  node.expression_type =
      makeAnonymousType(static_cast<int>(BuiltinType::Record), record_block);
}

void SemanticAnalyzer::visit(ast::EnumTypeSpecNode& node) {
  node.expression_type =
      makeAnonymousType(static_cast<int>(BuiltinType::Enumerated));

  int enum_value = 0;
  for (const auto& lit : node.literals) {
    if (sym_table.lookupCurrentScope(lit.lexeme)) {
      reportError("identifier '" + lit.lexeme + "' already declared", &lit);
      continue;
    }
    const int idx = sym_table.enterTab(lit.lexeme, ObjClass::Constant,
                                       node.expression_type, 0, 1, 1);
    auto& entry = sym_table.getTabEntry(idx);
    entry.adr = enum_value++;
  }
}

void SemanticAnalyzer::visit(ast::ConstDeclNode& node) {
  if (node.value == nullptr) {
    reportError("invalid constant definition for '" + node.identifier.lexeme +
                    "'",
                &node.identifier);
    return;
  }

  node.value->accept(*this);
  const int const_type = node.value->expression_type;
  const auto const_value = constantValue(*node.value);

  if (sym_table.lookupCurrentScope(node.identifier.lexeme)) {
    reportError("identifier '" + node.identifier.lexeme + "' already declared",
                &node.identifier);
    return;
  }

  node.tab_index = sym_table.enterTab(node.identifier.lexeme,
                                      ObjClass::Constant, const_type);
  if (const_value) {
    sym_table.getTabEntry(node.tab_index).adr = const_value->value;
  }
  node.expression_type = const_type;
}

void SemanticAnalyzer::visit(ast::ProgramNode& node) {
  if (auto existing = sym_table.lookup(node.identifier.lexeme)) {
    if (existing->obj != ObjClass::Type) {
      reportError("program name conflicts with existing identifier '" +
                      node.identifier.lexeme + "'",
                  &node.identifier);
    }
  } else {
    sym_table.enterTab(node.identifier.lexeme, ObjClass::Type,
                       get_base_type("integer"));
  }

  if (node.block == nullptr) return;

  for (const auto& decl : node.block->declarations) {
    decl->accept(*this);
  }

  enterScope();
  if (node.block->compound_stmt != nullptr) {
    node.block->compound_stmt->accept(*this);
  }
  leaveScope();
}

void SemanticAnalyzer::visit(ast::BlockNode& node) {
  enterScope();
  for (const auto& decl : node.declarations) {
    decl->accept(*this);
  }
  if (node.compound_stmt != nullptr) {
    node.compound_stmt->accept(*this);
  }
  leaveScope();
}

void SemanticAnalyzer::visit(ast::VarDeclNode& node) {
  auto* spec = dynamic_cast<ast::TypeSpecNode*>(node.type_spec.get());
  if (spec == nullptr) {
    reportError("invalid type specification in variable declaration",
                sourceToken(node));
    return;
  }

  const int var_type = resolveTypeSpec(*spec);
  node.expression_type = var_type;

  for (const auto& id : node.identifiers) {
    if (sym_table.lookupCurrentScope(id.lexeme)) {
      reportError("identifier '" + id.lexeme + "' already declared", &id);
      continue;
    }

    const int stack_size = variableStackSize(sym_table, var_type);
    sym_table.enterTab(id.lexeme, ObjClass::Variable, var_type, 0, 1,
                       stack_size);
  }
  // tab_index points to the first identifier registered for this declaration
  if (!node.identifiers.empty()) {
    if (auto entry = sym_table.lookup(node.identifiers[0].lexeme)) {
      node.tab_index = entry->idx;
    }
  }
}

void SemanticAnalyzer::visit(ast::TypeDeclNode& node) {
  auto* spec = dynamic_cast<ast::TypeSpecNode*>(node.type_def.get());
  if (spec == nullptr) {
    reportError("invalid type definition for '" + node.identifier.lexeme + "'",
                &node.identifier);
    return;
  }

  const int type_code = resolveTypeSpec(*spec);
  if (sym_table.lookupCurrentScope(node.identifier.lexeme)) {
    reportError("identifier '" + node.identifier.lexeme + "' already declared",
                &node.identifier);
    return;
  }

  int ref = 0;
  if (type_code >= RESERVED) {
    ref = sym_table.getTabEntry(type_code).ref;
  }

  node.tab_index = sym_table.enterTab(node.identifier.lexeme, ObjClass::Type,
                                      type_code, ref);
  node.expression_type = type_code;
}

void SemanticAnalyzer::visit(ast::ProcDeclNode& node) {
  if (sym_table.lookupCurrentScope(node.identifier.lexeme)) {
    reportError("identifier '" + node.identifier.lexeme + "' already declared",
                &node.identifier);
    return;
  }
  node.tab_index =
      sym_table.enterTab(node.identifier.lexeme, ObjClass::Procedure, 0);

  enterScope();
  sym_table.getTabEntry(node.tab_index).ref = sym_table.currentBlockIdx();

  for (const auto& param : node.parameters) {
    auto* spec = dynamic_cast<ast::TypeSpecNode*>(param->type_spec.get());
    const int param_type = spec != nullptr ? resolveTypeSpec(*spec) : 0;
    for (const auto& id : param->identifiers) {
      if (sym_table.lookupCurrentScope(id.lexeme)) {
        reportError("identifier '" + id.lexeme + "' already declared", &id);
        continue;
      }
      const int stack_size = variableStackSize(sym_table, param_type);
      sym_table.enterParam(id.lexeme, param_type, param->is_var ? 0 : 1,
                           stack_size);
    }
  }
  finalizeParamAddresses(sym_table.currentBlockIdx());
  if (node.block != nullptr) {
    for (const auto& decl : node.block->declarations) {
      decl->accept(*this);
    }
    if (node.block->compound_stmt != nullptr) {
      node.block->compound_stmt->accept(*this);
    }
  }
  leaveScope();
}

void SemanticAnalyzer::visit(ast::FuncDeclNode& node) {
  auto* ret_spec = dynamic_cast<ast::TypeSpecNode*>(node.return_type.get());
  const int return_type = ret_spec != nullptr ? resolveTypeSpec(*ret_spec) : 0;

  if (sym_table.lookupCurrentScope(node.identifier.lexeme)) {
    reportError("identifier '" + node.identifier.lexeme + "' already declared",
                &node.identifier);
    return;
  }
  node.tab_index = sym_table.enterTab(node.identifier.lexeme,
                                      ObjClass::Function, return_type);

  enterScope();
  sym_table.getTabEntry(node.tab_index).ref = sym_table.currentBlockIdx();

  for (const auto& param : node.parameters) {
    auto* spec = dynamic_cast<ast::TypeSpecNode*>(param->type_spec.get());
    const int param_type = spec != nullptr ? resolveTypeSpec(*spec) : 0;
    for (const auto& id : param->identifiers) {
      if (sym_table.lookupCurrentScope(id.lexeme)) {
        reportError("identifier '" + id.lexeme + "' already declared", &id);
        continue;
      }
      const int stack_size = variableStackSize(sym_table, param_type);
      sym_table.enterParam(id.lexeme, param_type, param->is_var ? 0 : 1,
                           stack_size);
    }
  }
  finalizeParamAddresses(sym_table.currentBlockIdx());
  if (node.block != nullptr) {
    for (const auto& decl : node.block->declarations) {
      decl->accept(*this);
    }
    if (node.block->compound_stmt != nullptr) {
      node.block->compound_stmt->accept(*this);
    }
  }
  leaveScope();
}

}  // namespace semantic
