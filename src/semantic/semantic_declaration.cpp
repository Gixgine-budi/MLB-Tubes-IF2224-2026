#include "semantic/semantic_analyzer.hpp"

#include <string>

#include "semantic/symtable_entries.hpp"

namespace semantic {

namespace {

std::string lower(std::string s) {
  for (char& c : s) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return s;
}

}  // namespace

int SemanticAnalyzer::resolveSimpleTypeName(const std::string& name) {
  const std::string key = lower(name);
  if (key == "integer") return BuiltinType::Integer;
  if (key == "real") return BuiltinType::Real;
  if (key == "boolean") return BuiltinType::Boolean;
  if (key == "char") return BuiltinType::Char;
  if (key == "string") return BuiltinType::String;

  if (auto entry = sym_table.lookup(name)) {
    if (entry->obj == ObjClass::Type) {
      return entry->type;
    }
  }
  reportError("unknown type '" + name + "'");
  return BuiltinType::Void;
}

int SemanticAnalyzer::resolveTypeSpec(ast::TypeSpecNode& spec) {
  spec.accept(*this);
  return spec.expression_type;
}

void SemanticAnalyzer::visit(ast::TypeSpecNode& node) {
  switch (node.kind) {
    case ast::TypeSpecNode::Kind::Simple:
      node.expression_type = resolveSimpleTypeName(node.name.lexeme);
      break;
    case ast::TypeSpecNode::Kind::Subrange:
      node.expression_type = BuiltinType::Subrange;
      if (node.low != nullptr) {
        node.low->accept(*this);
      }
      if (node.high != nullptr) {
        node.high->accept(*this);
      }
      break;
    case ast::TypeSpecNode::Kind::Array: {
      node.expression_type = BuiltinType::Array;
      if (node.index_type != nullptr) {
        resolveTypeSpec(*node.index_type);
      }
      if (node.element_type != nullptr) {
        resolveTypeSpec(*node.element_type);
      }
      break;
    }
    case ast::TypeSpecNode::Kind::Record:
      node.expression_type = BuiltinType::Record;
      for (auto& field : node.fields) {
        if (field.second != nullptr) {
          resolveTypeSpec(*field.second);
        }
      }
      break;
    case ast::TypeSpecNode::Kind::Enumerated:
      node.expression_type = BuiltinType::Enumerated;
      break;
  }
}

void SemanticAnalyzer::visit(ast::ProgramNode& node) {
  if (auto existing = sym_table.lookup(node.identifier.lexeme)) {
    if (existing->obj != ObjClass::Type) {
      reportError("program name conflicts with existing identifier '" +
                  node.identifier.lexeme + "'");
    }
  } else {
    sym_table.enterTab(node.identifier.lexeme, ObjClass::Type, BuiltinType::Program);
  }

  if (node.block == nullptr) {
    return;
  }

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
    reportError("invalid type specification in variable declaration");
    return;
  }

  const int var_type = resolveTypeSpec(*spec);

  for (const auto& id : node.identifiers) {
    if (auto existing = sym_table.lookup(id.lexeme)) {
      if (existing->obj == ObjClass::Variable ||
          existing->obj == ObjClass::Constant ||
          existing->obj == ObjClass::Procedure ||
          existing->obj == ObjClass::Function) {
        reportError("identifier '" + id.lexeme + "' already declared");
      }
    }
    const int idx = sym_table.enterTab(id.lexeme, ObjClass::Variable, var_type);
    node.tab_index = idx;
    node.expression_type = var_type;
  }
}

void SemanticAnalyzer::visit(ast::TypeDeclNode& node) {
  auto* spec = dynamic_cast<ast::TypeSpecNode*>(node.type_def.get());
  if (spec == nullptr) {
    reportError("invalid type definition for '" + node.identifier.lexeme + "'");
    return;
  }

  const int type_code = resolveTypeSpec(*spec);
  if (auto existing = sym_table.lookup(node.identifier.lexeme)) {
    if (existing->obj != ObjClass::Type) {
      reportError("identifier '" + node.identifier.lexeme +
                  "' already declared");
    }
  } else {
    node.tab_index =
        sym_table.enterTab(node.identifier.lexeme, ObjClass::Type, type_code);
  }
  node.expression_type = type_code;
}

void SemanticAnalyzer::visit(ast::ProcDeclNode& node) {
  if (sym_table.lookup(node.identifier.lexeme)) {
    reportError("identifier '" + node.identifier.lexeme + "' already declared");
  }
  sym_table.enterTab(node.identifier.lexeme, ObjClass::Procedure,
                     BuiltinType::Void);

  enterScope();
  for (const auto& param : node.parameters) {
    auto* spec = dynamic_cast<ast::TypeSpecNode*>(param->type_spec.get());
    const int param_type =
        spec != nullptr ? resolveTypeSpec(*spec) : BuiltinType::Void;
    for (const auto& id : param->identifiers) {
      sym_table.enterTab(id.lexeme, ObjClass::Variable, param_type, 0,
                         param->is_var ? 0 : 1);
    }
  }
  if (node.block != nullptr) {
    node.block->accept(*this);
  }
  leaveScope();
}

void SemanticAnalyzer::visit(ast::FuncDeclNode& node) {
  auto* ret_spec = dynamic_cast<ast::TypeSpecNode*>(node.return_type.get());
  const int return_type =
      ret_spec != nullptr ? resolveTypeSpec(*ret_spec) : BuiltinType::Void;

  if (sym_table.lookup(node.identifier.lexeme)) {
    reportError("identifier '" + node.identifier.lexeme + "' already declared");
  }
  sym_table.enterTab(node.identifier.lexeme, ObjClass::Function, return_type);

  enterScope();
  for (const auto& param : node.parameters) {
    auto* spec = dynamic_cast<ast::TypeSpecNode*>(param->type_spec.get());
    const int param_type =
        spec != nullptr ? resolveTypeSpec(*spec) : BuiltinType::Void;
    for (const auto& id : param->identifiers) {
      sym_table.enterTab(id.lexeme, ObjClass::Variable, param_type, 0,
                         param->is_var ? 0 : 1);
    }
  }
  if (node.block != nullptr) {
    node.block->accept(*this);
  }
  leaveScope();
}

}  // namespace semantic
