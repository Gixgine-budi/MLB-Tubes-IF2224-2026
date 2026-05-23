#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "lexer/token.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"
#include "semantic/sdt_internal.hpp"

namespace semantic {

namespace {

using lexer::TokenType;

Ptr<ast::ExprNode> buildVariableAccessLocal(const parser::ParseNode& node,
                                            SDTBuilder& builder);
Ptr<ast::ExprNode> buildFactorLocal(SDTBuilder& builder,
                                    const parser::ParseNode& node);
Ptr<ast::ExprNode> buildTermLocal(SDTBuilder& builder,
                                  const parser::ParseNode& node);
Ptr<ast::ExprNode> buildSimpleExpressionLocal(SDTBuilder& builder,
                                              const parser::ParseNode& node);
Ptr<ast::ExprNode> buildExpressionLocal(SDTBuilder& builder,
                                        const parser::ParseNode& node);

Ptr<ast::ExprNode> buildCaseConstantExpr(const parser::ParseNode& node) {
  if (node.type() != parser::NodeType::Constant || node.children().empty()) {
    return nullptr;
  }

  const auto& children = node.children();
  const auto& first = children[0];
  if (first->type() != parser::NodeType::TokenNode || !first->token()) {
    return nullptr;
  }

  const lexer::Token tok = first->token().value();
  if ((tok.type == TokenType::PLUS || tok.type == TokenType::MINUS) &&
      children.size() >= 2 &&
      children[1]->type() == parser::NodeType::TokenNode &&
      children[1]->token()) {
    const lexer::Token body = children[1]->token().value();
    Ptr<ast::ExprNode> base;
    if (body.type == TokenType::INTCON || body.type == TokenType::REALCON) {
      base = std::make_unique<ast::NumberNode>(body,
                                               body.type == TokenType::REALCON);
    } else if (body.type == TokenType::CHARCON ||
               body.type == TokenType::STRING) {
      base = std::make_unique<ast::StringNode>(body);
    } else {
      base = std::make_unique<ast::IdentNode>(body);
    }
    return std::make_unique<ast::UnaryOpNode>(tok, std::move(base));
  }

  if (tok.type == TokenType::INTCON || tok.type == TokenType::REALCON) {
    return std::make_unique<ast::NumberNode>(tok,
                                             tok.type == TokenType::REALCON);
  }
  if (tok.type == TokenType::CHARCON || tok.type == TokenType::STRING) {
    return std::make_unique<ast::StringNode>(tok);
  }
  return std::make_unique<ast::IdentNode>(tok);
}

Ptr<ast::ExprNode> buildIndexExpr(const parser::ParseNode& index_list,
                                  SDTBuilder& builder) {
  for (const auto& child : index_list.children()) {
    if (child->type() == parser::NodeType::Expression) {
      return buildExpressionLocal(builder, *child);
    }
    if (auto tok = sdt::tokenOf(*child)) {
      if (tok->type == TokenType::INTCON || tok->type == TokenType::CHARCON ||
          tok->type == TokenType::IDENT) {
        if (tok->type == TokenType::INTCON) {
          return std::make_unique<ast::NumberNode>(*tok, false);
        }
        if (tok->type == TokenType::CHARCON) {
          return std::make_unique<ast::StringNode>(*tok);
        }
        return std::make_unique<ast::IdentNode>(*tok);
      }
    }
  }
  return nullptr;
}

Ptr<ast::ExprNode> buildFactorLocal(SDTBuilder& builder,
                                    const parser::ParseNode& node) {
  const auto& children = node.children();
  if (!children.empty()) {
    if (auto tok = sdt::tokenOf(*children.front())) {
      if (tok->type == TokenType::NOTSY && children.size() >= 2) {
        auto operand = buildFactorLocal(builder, *children.back());
        if (operand != nullptr) {
          return std::make_unique<ast::UnaryOpNode>(*tok, std::move(operand));
        }
      }
    }
  }

  for (const auto& child : children) {
    if (auto tok = sdt::tokenOf(*child)) {
      switch (tok->type) {
        case TokenType::INTCON:
          return std::make_unique<ast::NumberNode>(*tok, false);
        case TokenType::REALCON:
          return std::make_unique<ast::NumberNode>(*tok, true);
        case TokenType::CHARCON:
        case TokenType::STRING:
          return std::make_unique<ast::StringNode>(*tok);
        default:
          break;
      }
    }

    switch (child->type()) {
      case parser::NodeType::Expression:
        return buildExpressionLocal(builder, *child);
      case parser::NodeType::FunctionCall: {
        lexer::Token name{};
        std::vector<Ptr<ast::ExprNode>> args;
        for (const auto& part : child->children()) {
          if (auto tok = sdt::tokenOf(*part)) {
            if (tok->type == TokenType::IDENT && name.lexeme.empty()) {
              name = *tok;
            }
          } else if (part->type() == parser::NodeType::ParameterList) {
            for (const auto& expr_child : part->children()) {
              if (expr_child->type() == parser::NodeType::Expression) {
                if (auto arg = buildExpressionLocal(builder, *expr_child)) {
                  args.push_back(std::move(arg));
                }
              }
            }
          }
        }
        if (!name.lexeme.empty()) {
          return std::make_unique<ast::FuncCallNode>(name, std::move(args));
        }
        break;
      }
      case parser::NodeType::Variable:
        return buildVariableAccessLocal(*child, builder);
      default:
        break;
    }
  }

  return nullptr;
}

Ptr<ast::ExprNode> buildTermLocal(SDTBuilder& builder,
                                  const parser::ParseNode& node) {
  std::vector<const parser::ParseNode*> parts;
  for (const auto& child : node.children()) {
    if (child->type() == parser::NodeType::Factor ||
        child->type() == parser::NodeType::MultiplicativeOperator) {
      parts.push_back(child.get());
    }
  }

  if (parts.empty()) {
    return nullptr;
  }

  Ptr<ast::ExprNode> result = buildFactorLocal(builder, *parts.front());
  for (size_t i = 1; i + 1 < parts.size(); i += 2) {
    const auto* op_node = parts[i];
    const auto* rhs_node = parts[i + 1];
    lexer::Token op{};
    if (const auto* op_tok =
            sdt::findChild(*op_node, parser::NodeType::TokenNode)) {
      if (auto tok = sdt::tokenOf(*op_tok)) {
        op = *tok;
      }
    }
    auto rhs = buildFactorLocal(builder, *rhs_node);
    if (result != nullptr && rhs != nullptr) {
      result = std::make_unique<ast::BinOpNode>(std::move(result), op,
                                                std::move(rhs));
    }
  }
  return result;
}

Ptr<ast::ExprNode> buildSimpleExpressionLocal(SDTBuilder& builder,
                                              const parser::ParseNode& node) {
  Ptr<ast::ExprNode> result;
  size_t idx = 0;
  const auto& children = node.children();

  if (!children.empty() &&
      children.front()->type() == parser::NodeType::TokenNode) {
    if (auto tok = sdt::tokenOf(*children.front())) {
      if (tok->type == TokenType::PLUS || tok->type == TokenType::MINUS) {
        idx = 1;
      }
    }
  }

  for (; idx < children.size(); ++idx) {
    if (children[idx]->type() == parser::NodeType::Term) {
      result = buildTermLocal(builder, *children[idx]);
      break;
    }
  }

  for (; idx < children.size(); ++idx) {
    if (children[idx]->type() == parser::NodeType::AdditiveOperator &&
        idx + 1 < children.size() &&
        children[idx + 1]->type() == parser::NodeType::Term) {
      lexer::Token op{};
      if (const auto* op_tok =
              sdt::findChild(*children[idx], parser::NodeType::TokenNode)) {
        if (auto tok = sdt::tokenOf(*op_tok)) {
          op = *tok;
        }
      }
      auto rhs = buildTermLocal(builder, *children[idx + 1]);
      if (result != nullptr && rhs != nullptr) {
        result = std::make_unique<ast::BinOpNode>(std::move(result), op,
                                                  std::move(rhs));
      }
      ++idx;
    }
  }

  return result;
}

Ptr<ast::ExprNode> buildExpressionLocal(SDTBuilder& builder,
                                        const parser::ParseNode& node) {
  const auto* simple = sdt::findChild(node, parser::NodeType::SimpleExpression);
  if (simple == nullptr) {
    return nullptr;
  }

  auto left = buildSimpleExpressionLocal(builder, *simple);
  const auto* rel = sdt::findChild(node, parser::NodeType::RelationalOperator);
  if (rel == nullptr) {
    return left;
  }

  const auto right_simples =
      sdt::childrenOfType(node, parser::NodeType::SimpleExpression);
  if (right_simples.size() < 2) {
    return left;
  }

  lexer::Token op{};
  if (const auto* op_tok = sdt::findChild(*rel, parser::NodeType::TokenNode)) {
    if (auto tok = sdt::tokenOf(*op_tok)) {
      op = *tok;
    }
  }

  auto right = buildSimpleExpressionLocal(builder, *right_simples.back());
  if (left != nullptr && right != nullptr) {
    return std::make_unique<ast::BinOpNode>(std::move(left), op,
                                            std::move(right));
  }
  return left;
}

Ptr<ast::ExprNode> buildVariableAccessLocal(const parser::ParseNode& node,
                                            SDTBuilder& builder) {
  Ptr<ast::ExprNode> current;
  for (const auto& child : node.children()) {
    if (auto tok = sdt::tokenOf(*child)) {
      if (tok->type == TokenType::IDENT && current == nullptr) {
        current = std::make_unique<ast::IdentNode>(*tok);
      }
    } else if (child->type() == parser::NodeType::ComponentVariable) {
      const auto* index_list =
          sdt::findChild(*child, parser::NodeType::IndexList);
      if (index_list != nullptr) {
        std::vector<Ptr<ast::ExprNode>> indices;
        if (auto idx = buildIndexExpr(*index_list, builder)) {
          indices.push_back(std::move(idx));
        }
        for (const auto& part : index_list->children()) {
          if (part->type() == parser::NodeType::Expression) {
            if (auto idx = buildExpressionLocal(builder, *part)) {
              indices.push_back(std::move(idx));
            }
          }
        }
        if (current != nullptr) {
          current = std::make_unique<ast::ArrayAccessNode>(std::move(current),
                                                           std::move(indices));
        }
      } else {
        for (const auto& part : child->children()) {
          if (auto tok = sdt::tokenOf(*part)) {
            if (tok->type == TokenType::IDENT && current != nullptr) {
              current = std::make_unique<ast::RecordAccessNode>(
                  std::move(current), *tok);
            }
          }
        }
      }
    }
  }
  return current;
}

}  // namespace

Ptr<ast::ExprNode> SDTBuilder::buildVariableAccess(
    const parser::ParseNode& node) {
  return buildVariableAccessLocal(node, *this);
}

Ptr<ast::StmtNode> SDTBuilder::buildStatement(const parser::ParseNode& node) {
  if (node.type() != parser::NodeType::Statement) {
    return nullptr;
  }

  for (const auto& child : node.children()) {
    switch (child->type()) {
      case parser::NodeType::AssignmentStatement:
        return buildAssign(*child);
      case parser::NodeType::IfStatement:
        return buildIf(*child);
      case parser::NodeType::CaseStatement:
        return buildCase(*child);
      case parser::NodeType::WhileStatement:
        return buildWhile(*child);
      case parser::NodeType::RepeatStatement:
        return buildRepeat(*child);
      case parser::NodeType::ForStatement:
        return buildFor(*child);
      case parser::NodeType::FunctionCall:
        return buildProcCall(*child);
      case parser::NodeType::CompoundStatement:
        return buildCompoundStmt(*child);
      default:
        break;
    }
  }
  return nullptr;
}

Ptr<ast::CompoundStmtNode> SDTBuilder::buildCompoundStmt(
    const parser::ParseNode& node) {
  std::vector<Ptr<ast::StmtNode>> statements;
  const auto* list = sdt::findChild(node, parser::NodeType::StatementList);
  if (list == nullptr) {
    return std::make_unique<ast::CompoundStmtNode>(std::move(statements));
  }

  for (const auto& child : list->children()) {
    if (child->type() == parser::NodeType::Statement) {
      if (auto stmt = buildStatement(*child)) {
        statements.push_back(std::move(stmt));
      }
    }
  }
  return std::make_unique<ast::CompoundStmtNode>(std::move(statements));
}

Ptr<ast::AssignNode> SDTBuilder::buildAssign(const parser::ParseNode& node) {
  const auto* var = sdt::findChild(node, parser::NodeType::Variable);
  const auto* expr = sdt::findChild(node, parser::NodeType::Expression);
  if (var == nullptr || expr == nullptr) {
    return nullptr;
  }
  auto target = buildVariableAccessLocal(*var, *this);
  auto value = buildExpressionLocal(*this, *expr);
  if (target == nullptr || value == nullptr) {
    return nullptr;
  }
  return std::make_unique<ast::AssignNode>(std::move(target), std::move(value));
}

Ptr<ast::IfNode> SDTBuilder::buildIf(const parser::ParseNode& node) {
  const auto* cond = sdt::findChild(node, parser::NodeType::Expression);
  if (cond == nullptr) {
    return nullptr;
  }
  auto condition = buildExpressionLocal(*this, *cond);
  if (condition == nullptr) {
    return nullptr;
  }

  Ptr<ast::StmtNode> then_branch;
  Ptr<ast::StmtNode> else_branch;
  bool seen_then = false;
  for (const auto& child : node.children()) {
    if (child->type() == parser::NodeType::Statement) {
      if (!seen_then) {
        then_branch = buildStatement(*child);
        seen_then = true;
      } else {
        else_branch = buildStatement(*child);
      }
    }
  }

  if (then_branch == nullptr) {
    return nullptr;
  }
  return std::make_unique<ast::IfNode>(
      std::move(condition), std::move(then_branch), std::move(else_branch));
}

Ptr<ast::StmtNode> SDTBuilder::buildCase(const parser::ParseNode& node) {
  const auto* selector = sdt::findChild(node, parser::NodeType::Expression);
  const auto* first_case = sdt::findChild(node, parser::NodeType::CaseBlock);
  if (selector == nullptr || first_case == nullptr) {
    return nullptr;
  }

  std::function<Ptr<ast::StmtNode>(const parser::ParseNode&)> lower_case_block;
  lower_case_block =
      [&](const parser::ParseNode& case_block) -> Ptr<ast::StmtNode> {
    std::vector<Ptr<ast::ExprNode>> labels;
    Ptr<ast::StmtNode> then_stmt;
    Ptr<ast::StmtNode> else_stmt;

    for (const auto& child : case_block.children()) {
      if (child->type() == parser::NodeType::Constant) {
        if (auto expr = buildCaseConstantExpr(*child)) {
          labels.push_back(std::move(expr));
        }
      } else if (child->type() == parser::NodeType::Statement &&
                 then_stmt == nullptr) {
        then_stmt = buildStatement(*child);
      } else if (child->type() == parser::NodeType::CaseBlock) {
        else_stmt = lower_case_block(*child);
      }
    }

    if (then_stmt == nullptr) {
      return else_stmt;
    }

    Ptr<ast::ExprNode> condition;
    const lexer::Token eql_tok = lexer::Token(TokenType::EQL, "=");
    const lexer::Token or_tok = lexer::Token(TokenType::ORSY, "or");

    for (auto& label : labels) {
      auto lhs = buildExpressionLocal(*this, *selector);
      if (lhs == nullptr || label == nullptr) {
        continue;
      }

      auto equals_expr = std::make_unique<ast::BinOpNode>(
          std::move(lhs), eql_tok, std::move(label));
      if (condition == nullptr) {
        condition = std::move(equals_expr);
      } else {
        condition = std::make_unique<ast::BinOpNode>(
            std::move(condition), or_tok, std::move(equals_expr));
      }
    }

    if (condition == nullptr) {
      return then_stmt;
    }

    return std::make_unique<ast::IfNode>(
        std::move(condition), std::move(then_stmt), std::move(else_stmt));
  };

  return lower_case_block(*first_case);
}

Ptr<ast::WhileNode> SDTBuilder::buildWhile(const parser::ParseNode& node) {
  const auto* cond = sdt::findChild(node, parser::NodeType::Expression);
  const auto* body = sdt::findChild(node, parser::NodeType::CompoundStatement);
  if (cond == nullptr || body == nullptr) {
    return nullptr;
  }
  auto condition = buildExpressionLocal(*this, *cond);
  auto compound = buildCompoundStmt(*body);
  if (condition == nullptr || compound == nullptr) {
    return nullptr;
  }
  return std::make_unique<ast::WhileNode>(std::move(condition),
                                          std::move(compound));
}

Ptr<ast::RepeatNode> SDTBuilder::buildRepeat(const parser::ParseNode& node) {
  std::vector<Ptr<ast::StmtNode>> statements;
  const auto* list = sdt::findChild(node, parser::NodeType::StatementList);
  const auto* cond = sdt::findChild(node, parser::NodeType::Expression);
  if (list != nullptr) {
    for (const auto& child : list->children()) {
      if (child->type() == parser::NodeType::Statement) {
        if (auto stmt = buildStatement(*child)) {
          statements.push_back(std::move(stmt));
        }
      }
    }
  }
  if (cond == nullptr) {
    return nullptr;
  }
  auto condition = buildExpressionLocal(*this, *cond);
  if (condition == nullptr) {
    return nullptr;
  }
  return std::make_unique<ast::RepeatNode>(std::move(statements),
                                           std::move(condition));
}

Ptr<ast::ForNode> SDTBuilder::buildFor(const parser::ParseNode& node) {
  lexer::Token iter{};
  Ptr<ast::ExprNode> initial;
  Ptr<ast::ExprNode> final_expr;
  bool is_downto = false;

  std::vector<const parser::ParseNode*> expressions;
  for (const auto& child : node.children()) {
    if (auto tok = sdt::tokenOf(*child)) {
      if (tok->type == TokenType::IDENT && iter.lexeme.empty()) {
        iter = *tok;
      } else if (tok->type == TokenType::DOWNTOSY) {
        is_downto = true;
      }
    } else if (child->type() == parser::NodeType::Expression) {
      expressions.push_back(child.get());
    }
  }

  if (expressions.size() >= 2) {
    initial = buildExpressionLocal(*this, *expressions[0]);
    final_expr = buildExpressionLocal(*this, *expressions[1]);
  }

  const auto* body = sdt::findChild(node, parser::NodeType::CompoundStatement);
  if (iter.lexeme.empty() || initial == nullptr || final_expr == nullptr ||
      body == nullptr) {
    return nullptr;
  }

  auto compound = buildCompoundStmt(*body);
  if (compound == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::ForNode>(iter, std::move(initial),
                                        std::move(final_expr), is_downto,
                                        std::move(compound));
}

Ptr<ast::ProcCallNode> SDTBuilder::buildProcCall(
    const parser::ParseNode& node) {
  lexer::Token name{};
  std::vector<Ptr<ast::ExprNode>> args;

  for (const auto& child : node.children()) {
    if (auto tok = sdt::tokenOf(*child)) {
      if (tok->type == TokenType::IDENT && name.lexeme.empty()) {
        name = *tok;
      }
    } else if (child->type() == parser::NodeType::ParameterList) {
      for (const auto& expr_child : child->children()) {
        if (expr_child->type() == parser::NodeType::Expression) {
          if (auto arg = buildExpressionLocal(*this, *expr_child)) {
            args.push_back(std::move(arg));
          }
        }
      }
    }
  }

  if (name.lexeme.empty()) {
    return nullptr;
  }
  return std::make_unique<ast::ProcCallNode>(name, std::move(args));
}

}  // namespace semantic
