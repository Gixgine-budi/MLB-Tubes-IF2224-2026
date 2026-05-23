#include "ast/stmt_nodes.hpp"
#include "semantic/semantic_analyzer.hpp"
#include "semantic/symtable_entries.hpp"

namespace semantic {

void SemanticAnalyzer::visit(ast::AssignNode& node) {
  node.target->accept(*this);
  node.expr->accept(*this);

  const int target_type = node.target->expression_type;
  const int expr_type = node.expr->expression_type;

  if (target_type == 0 || expr_type == 0) return;

  if (!isAssignmentCompatible(target_type, expr_type)) {
    reportError("assignment type mismatch");
  }
}

void SemanticAnalyzer::visit(ast::IfNode& node) {
  node.condition->accept(*this);
  if (node.condition->expression_type != get_base_type("boolean")) {
    reportError("if condition must be boolean");
  }
  node.then_branch->accept(*this);
  if (node.else_branch) node.else_branch->accept(*this);
}

void SemanticAnalyzer::visit(ast::WhileNode& node) {
  node.condition->accept(*this);
  if (node.condition->expression_type != get_base_type("boolean")) {
    reportError("while condition must be boolean");
  }
  node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::RepeatNode& node) {
  for (auto& stmt : node.statements) {
    stmt->accept(*this);
  }
  node.condition->accept(*this);
  if (node.condition->expression_type != get_base_type("boolean")) {
    reportError("repeat-until condition must be boolean");
  }
}

void SemanticAnalyzer::visit(ast::ForNode& node) {
  auto entry = sym_table.lookup(node.iterator.lexeme);
  if (!entry) {
    reportError("undeclared iterator variable '" + node.iterator.lexeme + "'");
  } else if (entry->type != get_base_type("integer")) {
    reportError("for-loop iterator must be integer");
  }

  node.initial->accept(*this);
  node.final->accept(*this);

  if (entry) {
    if (!isAssignmentCompatible(entry->type, node.initial->expression_type)) {
      reportError("for-loop initial value is not compatible with iterator type");
    }
    if (!isAssignmentCompatible(entry->type, node.final->expression_type)) {
      reportError("for-loop final value is not compatible with iterator type");
    }
  }

  node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::ProcCallNode& node) {
  auto entry = sym_table.lookup(node.id.lexeme);
  if (!entry) {
    // Tolerate known built-in procedures not yet in symbol table
    const std::string& name = node.id.lexeme;
    if (name != "writeln" && name != "readln" && name != "write" &&
        name != "read") {
      reportError("undeclared procedure '" + name + "'");
    }
  } else if (entry->obj != ObjClass::Procedure) {
    reportError("'" + node.id.lexeme + "' is not a procedure");
  } else {
    node.tab_index = entry->idx;
  }

  for (auto& arg : node.args) {
    arg->accept(*this);
  }
}

void SemanticAnalyzer::visit(ast::CompoundStmtNode& node) {
  for (auto& stmt : node.statements) {
    stmt->accept(*this);
  }
}

}  // namespace semantic
