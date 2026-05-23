#include "ast/expr_nodes.hpp"
#include "lexer/token.hpp"
#include "semantic/semantic_analyzer.hpp"
#include "semantic/symtable_entries.hpp"

namespace semantic {

void SemanticAnalyzer::visit(ast::NumberNode& node) {
  node.expression_type =
      node.is_real ? get_base_type("real") : get_base_type("integer");
}

void SemanticAnalyzer::visit(ast::StringNode& node) {
  node.expression_type = node.val.type == lexer::TokenType::CHARCON
                             ? get_base_type("char")
                             : get_base_type("string");
}

void SemanticAnalyzer::visit(ast::IdentNode& node) {
  auto entry = sym_table.lookup(node.id.lexeme);
  if (!entry) {
    reportError("undeclared identifier '" + node.id.lexeme + "'");
    node.expression_type = 0;
    return;
  }
  node.expression_type = entry->type;
  node.tab_index = entry->idx;
}

void SemanticAnalyzer::visit(ast::BinOpNode& node) {
  node.left->accept(*this);
  node.right->accept(*this);

  const int l = node.left->expression_type;
  const int r = node.right->expression_type;
  const int int_t = get_base_type("integer");
  const int real_t = get_base_type("real");
  const int bool_t = get_base_type("boolean");

  using T = lexer::TokenType;
  const T op = node.op.type;

  if (op == T::PLUS || op == T::MINUS || op == T::TIMES) {
    if ((l == int_t || l == real_t) && (r == int_t || r == real_t)) {
      node.expression_type = (l == real_t || r == real_t) ? real_t : int_t;
    } else {
      reportError("operands of '" + node.op.lexeme + "' must be numeric");
      node.expression_type = 0;
    }
  } else if (op == T::RDIV) {
    if ((l == int_t || l == real_t) && (r == int_t || r == real_t)) {
      node.expression_type = real_t;
    } else {
      reportError("operands of '/' must be numeric");
      node.expression_type = 0;
    }
  } else if (op == T::IDIV || op == T::IMOD) {
    if (l == int_t && r == int_t) {
      node.expression_type = int_t;
    } else {
      reportError("operands of '" + node.op.lexeme + "' must be integer");
      node.expression_type = 0;
    }
  } else if (op == T::EQL || op == T::NEQ || op == T::LSS || op == T::GTR ||
             op == T::LEQ || op == T::GEQ) {
    if (l == r ||
        ((l == int_t || l == real_t) && (r == int_t || r == real_t))) {
      node.expression_type = bool_t;
    } else {
      reportError("incompatible types for relational operator '" +
                  node.op.lexeme + "'");
      node.expression_type = 0;
    }
  } else if (op == T::ANDSY || op == T::ORSY) {
    if (l == bool_t && r == bool_t) {
      node.expression_type = bool_t;
    } else {
      reportError("operands of '" + node.op.lexeme + "' must be boolean");
      node.expression_type = 0;
    }
  } else {
    node.expression_type = 0;
  }
}

void SemanticAnalyzer::visit(ast::UnaryOpNode& node) {
  node.expr->accept(*this);
  using T = lexer::TokenType;
  const T op = node.op.type;

  if (op == T::NOTSY) {
    if (node.expr->expression_type != get_base_type("boolean")) {
      reportError("operand of 'not' must be boolean");
    }
    node.expression_type = get_base_type("boolean");
  } else if (op == T::PLUS || op == T::MINUS) {
    const int int_t = get_base_type("integer");
    const int real_t = get_base_type("real");
    const int t = node.expr->expression_type;
    if (t == int_t || t == real_t) {
      node.expression_type = t;
    } else {
      reportError("unary sign operator requires numeric operand");
      node.expression_type = 0;
    }
  }
}

void SemanticAnalyzer::visit(ast::FuncCallNode& node) {
  auto entry = sym_table.lookup(node.id.lexeme);
  if (!entry) {
    reportError("undeclared function '" + node.id.lexeme + "'");
    node.expression_type = 0;
  } else if (entry->obj != ObjClass::Function) {
    reportError("'" + node.id.lexeme + "' is not a function");
    node.expression_type = 0;
  } else {
    node.expression_type = entry->type;
    node.tab_index = entry->idx;
  }

  for (auto& arg : node.args) {
    arg->accept(*this);
  }
}

void SemanticAnalyzer::visit(ast::ArrayAccessNode& node) {
  node.array_expr->accept(*this);

  for (auto& idx : node.indices) {
    idx->accept(*this);
    if (idx->expression_type == get_base_type("real")) {
      reportError("array index must not be real");
    }
  }

  const int array_type_idx = node.array_expr->expression_type;
  if (array_type_idx > 0) {
    const auto& type_entry = sym_table.getTabEntry(array_type_idx);
    if (type_entry.ref > 0) {
      const auto& array_info = sym_table.getAtabEntry(type_entry.ref);
      node.expression_type = array_info.etyp;
      return;
    }
  }

  reportError("invalid array access");
  node.expression_type = 0;
}

void SemanticAnalyzer::visit(ast::RecordAccessNode& node) {
  node.record_expr->accept(*this);

  const int record_type_idx = node.record_expr->expression_type;
  if (record_type_idx > 0) {
    const auto& type_entry = sym_table.getTabEntry(record_type_idx);
    if (type_entry.ref > 0) {
      const auto& block_info = sym_table.getBtabEntry(type_entry.ref);
      int current_link = block_info.last;
      while (current_link > 0) {
        const auto& field_entry = sym_table.getTabEntry(current_link);
        if (field_entry.id == node.field.lexeme) {
          node.expression_type = field_entry.type;
          return;
        }
        current_link = field_entry.link;
      }
      reportError("record has no field '" + node.field.lexeme + "'");
    }
  }
  node.expression_type = 0;
}

}  // namespace semantic
