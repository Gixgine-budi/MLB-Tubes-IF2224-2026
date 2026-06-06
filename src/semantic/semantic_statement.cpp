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
    reportError("assignment type mismatch", sourceToken(*node.expr));
  } else {
    checkSubrangeAssignment(target_type, *node.expr, "assigned value");
  }
}

void SemanticAnalyzer::visit(ast::IfNode& node) {
  node.condition->accept(*this);
  if (!isBooleanLike(node.condition->expression_type)) {
    reportError("if condition must be boolean", sourceToken(*node.condition));
  }
  node.then_branch->accept(*this);
  if (node.else_branch) node.else_branch->accept(*this);
}

void SemanticAnalyzer::visit(ast::WhileNode& node) {
  node.condition->accept(*this);
  if (!isBooleanLike(node.condition->expression_type)) {
    reportError("while condition must be boolean",
                sourceToken(*node.condition));
  }
  node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::RepeatNode& node) {
  for (auto& stmt : node.statements) {
    stmt->accept(*this);
  }
  node.condition->accept(*this);
  if (!isBooleanLike(node.condition->expression_type)) {
    reportError("repeat-until condition must be boolean",
                sourceToken(*node.condition));
  }
}

void SemanticAnalyzer::visit(ast::ForNode& node) {
  auto entry = sym_table.lookup(node.iterator.lexeme);
  if (!entry) {
    reportError("undeclared iterator variable '" + node.iterator.lexeme + "'",
                &node.iterator);
  } else if (!isOrdinalLike(entry->type)) {
    reportError("for-loop iterator must be an ordinal type", &node.iterator);
  }

  node.initial->accept(*this);
  node.final->accept(*this);

  if (entry) {
    if (!isAssignmentCompatible(entry->type, node.initial->expression_type)) {
      reportError(
          "for-loop initial value is not compatible with iterator type",
          sourceToken(*node.initial));
    } else {
      checkSubrangeAssignment(entry->type, *node.initial,
                              "for-loop initial value");
    }
    if (!isAssignmentCompatible(entry->type, node.final->expression_type)) {
      reportError("for-loop final value is not compatible with iterator type",
                  sourceToken(*node.final));
    } else {
      checkSubrangeAssignment(entry->type, *node.final, "for-loop final value");
    }
  }

  node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::ProcCallNode& node) {
  auto entry = sym_table.lookup(node.id.lexeme);
  if (!entry) {
    reportError("undeclared procedure '" + node.id.lexeme + "'", &node.id);
  } else if (entry->obj != ObjClass::Procedure) {
    reportError("'" + node.id.lexeme + "' is not a procedure", &node.id);
  } else {
    node.tab_index = entry->idx;
    const std::string& name = node.id.lexeme;
    const bool predefined_io = name == "writeln" || name == "write" ||
                               name == "readln" || name == "read";
    if (!predefined_io) {
      const int expected = countFormalParams(entry->idx);
      if (static_cast<int>(node.args.size()) != expected) {
        reportError("procedure '" + name + "' expects " +
                        std::to_string(expected) + " argument(s), got " +
                        std::to_string(node.args.size()),
                    &node.id);
      }
    }
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
