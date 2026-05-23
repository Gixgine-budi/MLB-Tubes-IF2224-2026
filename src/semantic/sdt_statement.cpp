#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "lexer/token.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {

namespace {

using lexer::TokenType;

bool isToken(const parser::ParseNode& node) {
  return node.type() == parser::NodeType::TokenNode && node.token().has_value();
}

std::optional<lexer::Token> tokenOf(const parser::ParseNode& node) {
  if (isToken(node)) {
    return *node.token();
  }
  return std::nullopt;
}

const parser::ParseNode* findChild(const parser::ParseNode& parent,
                                   parser::NodeType type) {
  for (const auto& child : parent.children()) {
    if (child->type() == type) {
      return child.get();
    }
  }
  return nullptr;
}

}  // namespace

Ptr<ast::ExprNode> SDTBuilder::buildCaseConstantExpr(
    const parser::ParseNode& node) {
  if (node.type() != parser::NodeType::Constant || node.children().empty()) {
    reportBuildError(node, "invalid case label constant node");
    return nullptr;
  }

  const auto& children = node.children();
  const auto& first = children[0];
  if (first->type() != parser::NodeType::TokenNode || !first->token()) {
    reportBuildError(node, "case label constant does not start with token");
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

Ptr<ast::ExprNode> SDTBuilder::buildVariableAccess(
    const parser::ParseNode& node) {
  Ptr<ast::ExprNode> current;

  for (const auto& child : node.children()) {
    if (auto tok = tokenOf(*child)) {
      if (tok->type == TokenType::IDENT && current == nullptr) {
        current = std::make_unique<ast::IdentNode>(*tok);
      }
      continue;
    }

    if (child->type() != parser::NodeType::ComponentVariable ||
        current == nullptr) {
      continue;
    }

    const auto* index_list = findChild(*child, parser::NodeType::IndexList);
    if (index_list != nullptr) {
      std::vector<Ptr<ast::ExprNode>> indices;
      for (const auto& part : index_list->children()) {
        if (part->type() == parser::NodeType::Expression) {
          if (auto idx = buildExpression(*part)) {
            indices.push_back(std::move(idx));
          }
          continue;
        }

        if (auto tok = tokenOf(*part)) {
          if (tok->type == TokenType::INTCON) {
            indices.push_back(std::make_unique<ast::NumberNode>(*tok, false));
          } else if (tok->type == TokenType::CHARCON) {
            indices.push_back(std::make_unique<ast::StringNode>(*tok));
          } else if (tok->type == TokenType::IDENT) {
            indices.push_back(std::make_unique<ast::IdentNode>(*tok));
          }
        }
      }

      if (!indices.empty()) {
        current = std::make_unique<ast::ArrayAccessNode>(std::move(current),
                                                         std::move(indices));
      }
      continue;
    }

    for (const auto& part : child->children()) {
      if (auto tok = tokenOf(*part)) {
        if (tok->type == TokenType::IDENT) {
          current =
              std::make_unique<ast::RecordAccessNode>(std::move(current), *tok);
        }
      }
    }
  }

  if (current == nullptr) {
    reportBuildError(node, "failed to build variable access expression");
  }
  return current;
}

Ptr<ast::StmtNode> SDTBuilder::buildStatement(const parser::ParseNode& node) {
  if (node.type() != parser::NodeType::Statement) {
    reportBuildError(node, "expected Statement parse node");
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
  reportBuildError(node, "statement node does not contain a supported form");
  return nullptr;
}

Ptr<ast::CompoundStmtNode> SDTBuilder::buildCompoundStmt(
    const parser::ParseNode& node) {
  std::vector<Ptr<ast::StmtNode>> statements;
  const auto* list = findChild(node, parser::NodeType::StatementList);
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
  const auto* var = findChild(node, parser::NodeType::Variable);
  const auto* expr = findChild(node, parser::NodeType::Expression);
  if (var == nullptr || expr == nullptr) {
    reportBuildError(node,
                     "assignment statement is missing target or expression");
    return nullptr;
  }
  auto target = buildVariableAccess(*var);
  auto value = buildExpression(*expr);
  if (target == nullptr || value == nullptr) {
    reportBuildError(node,
                     "failed to build assignment target or value expression");
    return nullptr;
  }
  return std::make_unique<ast::AssignNode>(std::move(target), std::move(value));
}

Ptr<ast::IfNode> SDTBuilder::buildIf(const parser::ParseNode& node) {
  const auto* cond = findChild(node, parser::NodeType::Expression);
  if (cond == nullptr) {
    reportBuildError(node, "if statement is missing condition expression");
    return nullptr;
  }
  auto condition = buildExpression(*cond);
  if (condition == nullptr) {
    reportBuildError(node, "failed to build if-condition expression");
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
    reportBuildError(node, "if statement is missing then-branch statement");
    return nullptr;
  }
  return std::make_unique<ast::IfNode>(
      std::move(condition), std::move(then_branch), std::move(else_branch));
}

Ptr<ast::StmtNode> SDTBuilder::buildCase(const parser::ParseNode& node) {
  const auto* selector = findChild(node, parser::NodeType::Expression);
  const auto* first_case = findChild(node, parser::NodeType::CaseBlock);
  if (selector == nullptr || first_case == nullptr) {
    reportBuildError(node,
                     "case statement is missing selector or first case block");
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
      if (else_stmt == nullptr) {
        reportBuildError(case_block,
                         "case block has no statement branch to execute");
      }
      return else_stmt;
    }

    Ptr<ast::ExprNode> condition;
    const lexer::Token eql_tok = lexer::Token(TokenType::EQL, "=");
    const lexer::Token or_tok = lexer::Token(TokenType::ORSY, "or");

    for (auto& label : labels) {
      auto lhs = buildExpression(*selector);

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
  const auto* cond = findChild(node, parser::NodeType::Expression);
  const auto* body = findChild(node, parser::NodeType::CompoundStatement);
  if (cond == nullptr || body == nullptr) {
    reportBuildError(node,
                     "while statement is missing condition or body block");
    return nullptr;
  }
  auto condition = buildExpression(*cond);
  auto compound = buildCompoundStmt(*body);
  if (condition == nullptr || compound == nullptr) {
    reportBuildError(node,
                     "failed to build while-condition or while-body block");
    return nullptr;
  }
  return std::make_unique<ast::WhileNode>(std::move(condition),
                                          std::move(compound));
}

Ptr<ast::RepeatNode> SDTBuilder::buildRepeat(const parser::ParseNode& node) {
  std::vector<Ptr<ast::StmtNode>> statements;
  const auto* list = findChild(node, parser::NodeType::StatementList);
  const auto* cond = findChild(node, parser::NodeType::Expression);
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
    reportBuildError(node, "repeat statement is missing until condition");
    return nullptr;
  }
  auto condition = buildExpression(*cond);

  if (condition == nullptr) {
    reportBuildError(node, "failed to build repeat-until condition");
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
    if (auto tok = tokenOf(*child)) {
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
    initial = buildExpression(*expressions[0]);
    final_expr = buildExpression(*expressions[1]);
  }

  const auto* body = findChild(node, parser::NodeType::CompoundStatement);
  if (iter.lexeme.empty() || initial == nullptr || final_expr == nullptr ||
      body == nullptr) {
    reportBuildError(node,
                     "for statement is missing iterator, bounds, or "
                     "compound-statement body");
    return nullptr;
  }

  auto compound = buildCompoundStmt(*body);
  if (compound == nullptr) {
    reportBuildError(node, "failed to build for-loop body block");
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
    if (auto tok = tokenOf(*child)) {
      if (tok->type == TokenType::IDENT && name.lexeme.empty()) {
        name = *tok;
      }
    } else if (child->type() == parser::NodeType::ParameterList) {
      for (const auto& expr_child : child->children()) {
        if (expr_child->type() == parser::NodeType::Expression) {
          if (auto arg = buildExpression(*expr_child)) {
            args.push_back(std::move(arg));
          }
        }
      }
    }
  }

  if (name.lexeme.empty()) {
    reportBuildError(node, "procedure call is missing identifier token");
    return nullptr;
  }
  return std::make_unique<ast::ProcCallNode>(name, std::move(args));
}

}  // namespace semantic
