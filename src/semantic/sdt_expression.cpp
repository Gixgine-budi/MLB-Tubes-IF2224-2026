#include "lexer/token.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {
Ptr<ast::ExprNode> SDTBuilder::buildExpression(const parser::ParseNode &node) {
  if (node.children().empty()) {
    reportBuildError(node, "expression node has no children");
    return nullptr;
  }

  // expression -> simple_expr [ rel_op simple_expr ]
  auto left = buildSimpleExpression(*node.children()[0]);
  if (left == nullptr) {
    reportBuildError(node, "failed to build left side of expression");
    return nullptr;
  }

  if (node.children().size() == 3) {
    auto op_node = node.children()[1].get();
    if (op_node->children().empty() || !op_node->children()[0]->token()) {
      reportBuildError(node, "relational expression is missing operator token");
      return nullptr;
    }
    lexer::Token op = op_node->children()[0]->token().value();

    auto right = buildSimpleExpression(*node.children()[2]);
    if (right == nullptr) {
      reportBuildError(node, "failed to build right side of expression");
      return nullptr;
    }
    return std::make_unique<ast::BinOpNode>(std::move(left), op,
                                            std::move(right));
  }
  return left;
}

Ptr<ast::ExprNode> SDTBuilder::buildSimpleExpression(
    const parser::ParseNode &node) {
  if (node.children().empty()) {
    reportBuildError(node, "simple expression node has no children");
    return nullptr;
  }

  size_t i = 0;
  lexer::Token unary_op = {lexer::TokenType::INVALID,
                           lexer::InvalidType::NotInvalid, "", 0, 0};
  if (node.children()[0]->type() == parser::NodeType::TokenNode) {
    auto op_node = node.children()[0].get();
    unary_op = op_node->token().value();
    i++;
  }

  if (i >= node.children().size()) {
    reportBuildError(node, "simple expression is missing term operand");
    return nullptr;
  }

  auto term = buildTerm(*node.children()[i++]);
  if (term == nullptr) {
    reportBuildError(node, "failed to build term in simple expression");
    return nullptr;
  }
  Ptr<ast::ExprNode> current = std::move(term);

  if (unary_op.type != lexer::TokenType::INVALID) {
    current = std::make_unique<ast::UnaryOpNode>(unary_op, std::move(current));
  }

  while (i < node.children().size()) {
    auto op_node = node.children()[i++].get();
    if (op_node->children().empty() || !op_node->children()[0]->token()) {
      reportBuildError(node, "additive expression is missing operator token");
      return nullptr;
    }
    lexer::Token op = op_node->children()[0]->token().value();

    if (i >= node.children().size()) {
      reportBuildError(node, "additive expression is missing right-hand term");
      return nullptr;
    }

    auto next_term = buildTerm(*node.children()[i++]);
    if (next_term == nullptr) {
      reportBuildError(
          node, "failed to build right-hand term in additive expression");
      return nullptr;
    }
    current = std::make_unique<ast::BinOpNode>(std::move(current), op,
                                               std::move(next_term));
  }
  return current;
}

Ptr<ast::ExprNode> SDTBuilder::buildTerm(const parser::ParseNode &node) {
  if (node.children().empty()) {
    reportBuildError(node, "term node has no children");
    return nullptr;
  }

  auto current = buildFactor(*node.children()[0]);
  if (current == nullptr) {
    reportBuildError(node, "failed to build first factor in term");
    return nullptr;
  }
  size_t i = 1;

  while (i < node.children().size()) {
    auto op_node = node.children()[i++].get();
    if (op_node->children().empty() || !op_node->children()[0]->token()) {
      reportBuildError(node,
                       "multiplicative expression is missing operator token");
      return nullptr;
    }
    lexer::Token op = op_node->children()[0]->token().value();

    if (i >= node.children().size()) {
      reportBuildError(
          node, "multiplicative expression is missing right-hand factor");
      return nullptr;
    }

    auto next_factor = buildFactor(*node.children()[i++]);
    if (next_factor == nullptr) {
      reportBuildError(
          node,
          "failed to build right-hand factor in multiplicative expression");
      return nullptr;
    }
    current = std::make_unique<ast::BinOpNode>(std::move(current), op,
                                               std::move(next_factor));
  }
  return current;
}

Ptr<ast::ExprNode> SDTBuilder::buildFactor(const parser::ParseNode &node) {
  if (node.children().empty()) {
    reportBuildError(node, "factor node has no children");
    return nullptr;
  }

  auto &child = *node.children()[0];

  if (child.type() == parser::NodeType::TokenNode) {
    auto t = child.token().value();
    if (t.type == lexer::TokenType::INTCON ||
        t.type == lexer::TokenType::REALCON) {
      return std::make_unique<ast::NumberNode>(
          t, t.type == lexer::TokenType::REALCON);
    }
    if (t.type == lexer::TokenType::CHARCON ||
        t.type == lexer::TokenType::STRING) {
      return std::make_unique<ast::StringNode>(t);
    }
    if (t.type == lexer::TokenType::NOTSY) {
      if (node.children().size() < 2) {
        reportBuildError(node, "not factor is missing operand");
        return nullptr;
      }
      auto factor = buildFactor(*node.children()[1]);
      if (factor == nullptr) {
        reportBuildError(node, "failed to build operand of not factor");
        return nullptr;
      }
      return std::make_unique<ast::UnaryOpNode>(t, std::move(factor));
    }
    if (t.type == lexer::TokenType::LPARENT) {
      if (node.children().size() < 2) {
        reportBuildError(node, "parenthesized factor is missing expression");
        return nullptr;
      }
      return buildExpression(*node.children()[1]);
    }
  } else if (child.type() == parser::NodeType::Variable) {
    return buildVariableAccess(child);
  } else if (child.type() == parser::NodeType::FunctionCall) {
    auto id_tok = child.children()[0]->token().value();
    std::vector<Ptr<ast::ExprNode>> args;
    if (child.children().size() > 2 &&
        child.children()[2]->type() == parser::NodeType::ParameterList) {
      const auto &param_list = child.children()[2];
      for (const auto &p : param_list->children()) {
        if (p->type() == parser::NodeType::Expression) {
          args.push_back(buildExpression(*p));
        }
      }
    }
    return std::make_unique<ast::FuncCallNode>(id_tok, std::move(args));
  }

  reportBuildError(node, "unsupported factor form");
  return nullptr;
}

}  // namespace semantic