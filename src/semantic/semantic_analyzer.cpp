#include "semantic/semantic_analyzer.hpp"

#include <iostream>
#include <string>

#include "lexer/token.hpp"
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

bool isRelational(lexer::TokenType type) {
  using lexer::TokenType;
  return type == TokenType::EQL || type == TokenType::NEQ ||
         type == TokenType::GTR || type == TokenType::GEQ ||
         type == TokenType::LSS || type == TokenType::LEQ;
}

bool isArithmetic(lexer::TokenType type) {
  using lexer::TokenType;
  return type == TokenType::PLUS || type == TokenType::MINUS ||
         type == TokenType::TIMES || type == TokenType::RDIV ||
         type == TokenType::IDIV || type == TokenType::IMOD;
}

}  // namespace

void SemanticAnalyzer::analyze(ast::AstNode& root) { root.accept(*this); }

void SemanticAnalyzer::reportError(const std::string& message) {
  std::cerr << "semantic error: " << message << '\n';
  has_errors_ = true;
}

void SemanticAnalyzer::enterScope() { sym_table.pushBlock(); }

void SemanticAnalyzer::leaveScope() { sym_table.popBlock(); }

void SemanticAnalyzer::visitExpr(ast::ExprNode& expr) { expr.accept(*this); }

void SemanticAnalyzer::visitStmt(ast::StmtNode& stmt) { stmt.accept(*this); }

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

bool SemanticAnalyzer::typesCompatible(int left, int right) const {
  if (left == right) return true;
  if (left == BuiltinType::Integer && right == BuiltinType::Subrange) return true;
  if (left == BuiltinType::Subrange && right == BuiltinType::Integer) return true;
  if (left == BuiltinType::Subrange && right == BuiltinType::Subrange) return true;
  return false;
}

bool SemanticAnalyzer::assignmentCompatible(int target, int value) const {
  if (target == BuiltinType::Real && value == BuiltinType::Integer) return true;
  return typesCompatible(target, value);
}

void SemanticAnalyzer::checkTypeCompatibility(int expected_type, int actual_type,
                                              const std::string& context) {
  if (!typesCompatible(expected_type, actual_type)) {
    reportError("type mismatch in " + context);
  }
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
    const int param_type = spec != nullptr ? resolveTypeSpec(*spec) : BuiltinType::Void;
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
    const int param_type = spec != nullptr ? resolveTypeSpec(*spec) : BuiltinType::Void;
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

void SemanticAnalyzer::visit(ast::NumberNode& node) {
  node.expression_type =
      node.is_real ? BuiltinType::Real : BuiltinType::Integer;
}

void SemanticAnalyzer::visit(ast::StringNode& node) {
  node.expression_type = BuiltinType::String;
}

void SemanticAnalyzer::visit(ast::IdentNode& node) {
  const std::string key = lower(node.id.lexeme);
  if (key == "true" || key == "false") {
    node.expression_type = BuiltinType::Boolean;
    if (auto entry = sym_table.lookup(node.id.lexeme)) {
      node.tab_index = entry->idx;
    }
    return;
  }

  if (auto entry = sym_table.lookup(node.id.lexeme)) {
    node.tab_index = entry->idx;
    node.expression_type = entry->type;
    return;
  }

  reportError("undefined identifier '" + node.id.lexeme + "'");
  node.expression_type = BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::UnaryOpNode& node) {
  if (node.expr != nullptr) {
    visitExpr(*node.expr);
  }

  if (node.op.type == lexer::TokenType::NOTSY) {
    checkTypeCompatibility(BuiltinType::Boolean, node.expr->expression_type,
                         "not expression");
    node.expression_type = BuiltinType::Boolean;
    return;
  }

  if (node.op.type == lexer::TokenType::PLUS ||
      node.op.type == lexer::TokenType::MINUS) {
    node.expression_type = node.expr->expression_type;
  }
}

void SemanticAnalyzer::visit(ast::BinOpNode& node) {
  if (node.left != nullptr) {
    visitExpr(*node.left);
  }
  if (node.right != nullptr) {
    visitExpr(*node.right);
  }

  const auto op = node.op.type;
  if (isRelational(op)) {
    node.expression_type = BuiltinType::Boolean;
    return;
  }

  if (op == lexer::TokenType::ANDSY || op == lexer::TokenType::ORSY) {
    checkTypeCompatibility(BuiltinType::Boolean, node.left->expression_type,
                         "logical operator");
    checkTypeCompatibility(BuiltinType::Boolean, node.right->expression_type,
                         "logical operator");
    node.expression_type = BuiltinType::Boolean;
    return;
  }

  if (node.left->expression_type == BuiltinType::Real ||
      node.right->expression_type == BuiltinType::Real) {
    node.expression_type = BuiltinType::Real;
  } else {
    node.expression_type = BuiltinType::Integer;
  }

  if (isArithmetic(op) && node.expression_type == BuiltinType::Integer) {
    checkTypeCompatibility(BuiltinType::Integer, node.left->expression_type,
                           "binary expression");
    checkTypeCompatibility(BuiltinType::Integer, node.right->expression_type,
                           "binary expression");
  }
}

void SemanticAnalyzer::visit(ast::FuncCallNode& node) {
  if (auto entry = sym_table.lookup(node.id.lexeme)) {
    node.tab_index = entry->idx;
    node.expression_type = entry->type;
  } else {
    reportError("undefined function '" + node.id.lexeme + "'");
    node.expression_type = BuiltinType::Void;
  }

  for (const auto& arg : node.args) {
    if (arg != nullptr) {
      visitExpr(*arg);
    }
  }
}

void SemanticAnalyzer::visit(ast::ArrayAccessNode& node) {
  if (node.array_expr != nullptr) {
    visitExpr(*node.array_expr);
  }
  for (const auto& index : node.indices) {
    if (index != nullptr) {
      visitExpr(*index);
      checkTypeCompatibility(BuiltinType::Integer, index->expression_type,
                           "array index");
    }
  }

  if (node.array_expr != nullptr &&
      node.array_expr->expression_type == BuiltinType::Array &&
      node.array_expr->tab_index != 0) {
    const auto& base = sym_table.getTabEntry(node.array_expr->tab_index);
    if (base.ref >= 0) {
      const auto& arr = sym_table.getAtabEntry(base.ref);
      node.expression_type = arr.etyp;
      return;
    }
  }
  node.expression_type = node.array_expr != nullptr
                             ? node.array_expr->expression_type
                             : BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::RecordAccessNode& node) {
  if (node.record_expr != nullptr) {
    visitExpr(*node.record_expr);
  }
  node.expression_type = node.record_expr != nullptr
                             ? node.record_expr->expression_type
                             : BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::AssignNode& node) {
  if (node.target != nullptr) {
    visitExpr(*node.target);
  }
  if (node.expr != nullptr) {
    visitExpr(*node.expr);
  }

  if (node.target != nullptr && node.expr != nullptr &&
      !assignmentCompatible(node.target->expression_type,
                            node.expr->expression_type)) {
    reportError("assignment-incompatible types");
  }
  node.expression_type = BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::IfNode& node) {
  if (node.condition != nullptr) {
    visitExpr(*node.condition);
    checkTypeCompatibility(BuiltinType::Boolean, node.condition->expression_type,
                         "if condition");
  }
  if (node.then_branch != nullptr) {
    visitStmt(*node.then_branch);
  }
  if (node.else_branch != nullptr) {
    visitStmt(*node.else_branch);
  }
  node.expression_type = BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::WhileNode& node) {
  if (node.condition != nullptr) {
    visitExpr(*node.condition);
    checkTypeCompatibility(BuiltinType::Boolean, node.condition->expression_type,
                         "while condition");
  }
  if (node.body != nullptr) {
    visitStmt(*node.body);
  }
  node.expression_type = BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::RepeatNode& node) {
  for (const auto& stmt : node.statements) {
    if (stmt != nullptr) {
      visitStmt(*stmt);
    }
  }
  if (node.condition != nullptr) {
    visitExpr(*node.condition);
    checkTypeCompatibility(BuiltinType::Boolean, node.condition->expression_type,
                         "repeat-until condition");
  }
  node.expression_type = BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::ForNode& node) {
  if (auto existing = sym_table.lookup(node.iterator.lexeme)) {
    if (existing->obj != ObjClass::Variable) {
      reportError("for-loop variable '" + node.iterator.lexeme +
                  "' is not a variable");
    }
  } else {
    sym_table.enterTab(node.iterator.lexeme, ObjClass::Variable,
                       BuiltinType::Integer);
  }

  if (node.initial != nullptr) {
    visitExpr(*node.initial);
    checkTypeCompatibility(BuiltinType::Integer, node.initial->expression_type,
                         "for initial value");
  }
  if (node.final != nullptr) {
    visitExpr(*node.final);
    checkTypeCompatibility(BuiltinType::Integer, node.final->expression_type,
                         "for final value");
  }
  if (node.body != nullptr) {
    visitStmt(*node.body);
  }
  node.expression_type = BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::ProcCallNode& node) {
  if (auto entry = sym_table.lookup(node.id.lexeme)) {
    node.tab_index = entry->idx;
    if (entry->obj != ObjClass::Procedure && entry->obj != ObjClass::Function) {
      reportError("'" + node.id.lexeme + "' is not a procedure");
    }
  } else {
    reportError("undefined procedure '" + node.id.lexeme + "'");
  }

  for (const auto& arg : node.args) {
    if (arg != nullptr) {
      visitExpr(*arg);
    }
  }
  node.expression_type = BuiltinType::Void;
}

void SemanticAnalyzer::visit(ast::CompoundStmtNode& node) {
  for (const auto& stmt : node.statements) {
    if (stmt != nullptr) {
      visitStmt(*stmt);
    }
  }
  node.expression_type = BuiltinType::Void;
}

}  // namespace semantic
