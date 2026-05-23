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
  const auto existing = sym_table.lookup(node.iterator.lexeme);
  if (!existing) {
    reportError("undefined identifier '" + node.iterator.lexeme +
                "' in for statement");
  } else if (existing->obj != ObjClass::Variable) {
    reportError("for-loop control '" + node.iterator.lexeme +
                "' must be a variable");
  } else if (!typesCompatible(BuiltinType::Integer, existing->type)) {
    reportError("for-loop variable '" + node.iterator.lexeme +
                "' must have an integer-compatible type");
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
