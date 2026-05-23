#include "lexer/token.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {
Ptr<ast::ExprNode> SDTBuilder::buildExpression(const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  // expression -> simple_expr [ rel_op simple_expr ]
  auto left = buildSimpleExpression(*node.children()[0]);

  if (node.children().size() == 3) {
    auto op_node = node.children()[1].get();
    lexer::Token op = op_node->children()[0]->token().value();

    auto right = buildSimpleExpression(*node.children()[2]);
    return std::make_unique<ast::BinOpNode>(std::move(left), op,
                                            std::move(right));
  }
  return left;
}

Ptr<ast::ExprNode> SDTBuilder::buildSimpleExpression(
    const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  size_t i = 0;
  lexer::Token unary_op = {lexer::TokenType::INVALID,
                           lexer::InvalidType::NotInvalid, "", 0, 0};
  if (node.children()[0]->type() == parser::NodeType::TokenNode) {
    auto op_node = node.children()[0].get();
    unary_op = op_node->token().value();
    i++;
  }

  auto term = buildTerm(*node.children()[i++]);
  Ptr<ast::ExprNode> current = std::move(term);

  if (unary_op.type == lexer::TokenType::INVALID) {
    current = std::make_unique<ast::UnaryOpNode>(unary_op, std::move(current));
  }

  while (i < node.children().size()) {
    auto op_node = node.children()[i++].get();
    lexer::Token op = op_node->children()[0]->token().value();

    auto next_term = buildTerm(*node.children()[i++]);
    current = std::make_unique<ast::BinOpNode>(std::move(current), op,
                                               std::move(next_term));
  }
  return current;
}

Ptr<ast::ExprNode> SDTBuilder::buildTerm(const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  auto current = buildFactor(*node.children()[0]);
  size_t i = 1;

  while (i < node.children().size()) {
    auto op_node = node.children()[i++].get();
    lexer::Token op = op_node->children()[0]->token().value();

    auto next_factor = buildFactor(*node.children()[i++]);
    current = std::make_unique<ast::BinOpNode>(std::move(current), op,
                                               std::move(next_factor));
  }
  return current;
}

Ptr<ast::ExprNode> SDTBuilder::buildFactor(const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

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
      auto factor = buildFactor(*node.children()[1]);
      return std::make_unique<ast::UnaryOpNode>(t, std::move(factor));
    }
    if (t.type == lexer::TokenType::LPARENT) {
      return buildExpression(*node.children()[1]);
    }
  } else if (child.type() == parser::NodeType::Variable) {
    return buildVariableAccess(child);
  } else if (child.type() == parser::NodeType::FunctionCall) {
    auto id_tok = child.children()[0]->token().value();
    std::vector<Ptr<ast::ExprNode>> args;
    if (child.children().size() > 1 &&
        child.children()[1]->type() == parser::NodeType::ParameterList) {
      const auto &param_list = child.children()[1];
      for (const auto &p : param_list->children()) {
        if (p->type() == parser::NodeType::Expression) {
          args.push_back(buildExpression(*p));
        }
      }
    }
    return std::make_unique<ast::FuncCallNode>(id_tok, std::move(args));
  }

  return nullptr;
}

Ptr<ast::ExprNode> SDTBuilder::buildVariableAccess(
    const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  Ptr<ast::ExprNode> var_expr =
      std::make_unique<ast::IdentNode>(node.children()[0]->token().value());

  for (size_t i = 1; i < node.children().size(); ++i) {
    auto &comp = *node.children()[i];
    if (comp.type() == parser::NodeType::ComponentVariable) {
      auto &comp_children = comp.children();
      if (comp_children[0]->type() == parser::NodeType::TokenNode) {
        auto t = comp_children[0]->token().value().type;
        if (t == lexer::TokenType::LBRACK) {
          auto &idx_list = *comp_children[1];
          std::vector<Ptr<ast::ExprNode>> indices;
          for (const auto &expr_node : idx_list.children()) {
            if (expr_node->type() == parser::NodeType::Expression) {
              indices.push_back(buildExpression(*expr_node));
            }
          }
          var_expr = std::make_unique<ast::ArrayAccessNode>(std::move(var_expr),
                                                            std::move(indices));
        } else if (t == lexer::TokenType::PERIOD) {
          auto field_id = comp_children[1]->token().value();
          var_expr = std::make_unique<ast::RecordAccessNode>(
              std::move(var_expr), field_id);
        }
      }
    }
  }

  return var_expr;
}

}  // namespace semantic