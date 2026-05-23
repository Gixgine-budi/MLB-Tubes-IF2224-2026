#include "semantic/sdt_builder.hpp"

#include <memory>
#include <string>
#include <utility>

#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

namespace {

using lexer::TokenType;
using parser::NodeType;

const lexer::Token* first_token(const parser::ParseNode& node) {
  if (node.type() == NodeType::TokenNode && node.token()) {
    return &*node.token();
  }
  for (const auto& child : node.children()) {
    if (const auto* token = first_token(*child)) return token;
  }
  return nullptr;
}

bool is_literal(TokenType type) {
  return type == TokenType::INTCON || type == TokenType::REALCON ||
         type == TokenType::CHARCON || type == TokenType::STRING;
}

AstPtr make_literal(const lexer::Token& token) {
  auto literal = std::make_unique<AstNode>(AstKind::Literal, token);
  literal->setValue(token.lexeme);
  switch (token.type) {
    case TokenType::INTCON:
      literal->annotation().type = "integer";
      break;
    case TokenType::REALCON:
      literal->annotation().type = "real";
      break;
    case TokenType::CHARCON:
      literal->annotation().type = "char";
      break;
    case TokenType::STRING:
      literal->annotation().type = "string";
      break;
    default:
      break;
  }
  return literal;
}

AstPtr make_var_ref(const lexer::Token& token) {
  auto ref = std::make_unique<AstNode>(AstKind::VarRef, token);
  ref->setName(token.lexeme);
  return ref;
}

AstPtr make_unary(std::string op, AstPtr operand) {
  auto unary = std::make_unique<AstNode>(AstKind::UnaryOp);
  unary->setOp(std::move(op));
  unary->addChild(std::move(operand));
  return unary;
}

AstPtr make_binary(std::string op, AstPtr left, AstPtr right) {
  auto binary = std::make_unique<AstNode>(AstKind::BinOp);
  binary->setOp(std::move(op));
  binary->addChild(std::move(left));
  binary->addChild(std::move(right));
  return binary;
}

}  // namespace

AstPtr SdtBuilder::buildExpression(const parser::ParseNode& node) const {
  AstPtr left;
  std::string pending_op;

  for (const auto& child : node.children()) {
    if (child->type() == NodeType::SimpleExpression) {
      auto operand = buildSimpleExpression(*child);
      if (!left) {
        left = std::move(operand);
      } else if (!pending_op.empty()) {
        left = make_binary(std::move(pending_op), std::move(left),
                           std::move(operand));
      }
      continue;
    }

    if (child->type() == NodeType::RelationalOperator) {
      if (const auto* token = first_token(*child)) {
        pending_op = tokenOperator(*token);
      }
    }
  }

  return left ? std::move(left) : makeError();
}

AstPtr SdtBuilder::buildSimpleExpression(const parser::ParseNode& node) const {
  AstPtr left;
  std::string pending_op;
  std::string unary_sign;

  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && (token->type == TokenType::PLUS ||
                  token->type == TokenType::MINUS)) {
      unary_sign = tokenOperator(*token);
      continue;
    }

    if (child->type() == NodeType::AdditiveOperator) {
      if (const auto* token = first_token(*child)) {
        pending_op = tokenOperator(*token);
      }
      continue;
    }

    if (child->type() != NodeType::Term) continue;

    auto operand = buildTerm(*child);
    if (!unary_sign.empty()) {
      if (unary_sign == "-") {
        operand = make_unary("-", std::move(operand));
      }
      unary_sign.clear();
    }

    if (!left) {
      left = std::move(operand);
    } else if (!pending_op.empty()) {
      left = make_binary(std::move(pending_op), std::move(left),
                         std::move(operand));
    }
  }

  return left ? std::move(left) : makeError();
}

AstPtr SdtBuilder::buildTerm(const parser::ParseNode& node) const {
  AstPtr left;
  std::string pending_op;

  for (const auto& child : node.children()) {
    if (child->type() == NodeType::MultiplicativeOperator) {
      if (const auto* token = first_token(*child)) {
        pending_op = tokenOperator(*token);
      }
      continue;
    }

    if (child->type() != NodeType::Factor) continue;

    auto operand = buildFactor(*child);
    if (!left) {
      left = std::move(operand);
    } else if (!pending_op.empty()) {
      left = make_binary(std::move(pending_op), std::move(left),
                         std::move(operand));
    }
  }

  return left ? std::move(left) : makeError();
}

AstPtr SdtBuilder::buildFactor(const parser::ParseNode& node) const {
  bool saw_not = false;

  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child)) {
      if (token->type == TokenType::NOTSY) {
        saw_not = true;
        continue;
      }
      if (is_literal(token->type)) {
        auto literal = make_literal(*token);
        return saw_not ? make_unary("not", std::move(literal))
                       : std::move(literal);
      }
      if (token->type == TokenType::IDENT) {
        auto ref = make_var_ref(*token);
        return saw_not ? make_unary("not", std::move(ref)) : std::move(ref);
      }
      continue;
    }

    AstPtr result;
    switch (child->type()) {
      case NodeType::Expression:
        result = buildExpression(*child);
        break;
      case NodeType::Factor:
        result = buildFactor(*child);
        break;
      case NodeType::Variable:
        result = buildVariable(*child);
        break;
      case NodeType::FunctionCall:
        result = buildFunctionCall(*child);
        break;
      case NodeType::Error:
        result = makeError();
        break;
      default:
        break;
    }

    if (result) {
      return saw_not ? make_unary("not", std::move(result))
                     : std::move(result);
    }
  }

  return makeError();
}

AstPtr SdtBuilder::buildVariable(const parser::ParseNode& node) const {
  AstPtr base;

  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT && !base) {
      base = make_var_ref(*token);
      continue;
    }

    if (child->type() == NodeType::ComponentVariable) {
      if (!base) base = makeError();
      base = buildComponentVariable(std::move(base), *child);
    }
  }

  return base ? std::move(base) : makeError();
}

AstPtr SdtBuilder::buildComponentVariable(
    AstPtr base, const parser::ParseNode& node) const {
  bool in_index = false;

  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child)) {
      if (token->type == TokenType::LBRACK) {
        in_index = true;
        continue;
      }
      if (token->type == TokenType::RBRACK) {
        in_index = false;
        continue;
      }
      if (token->type == TokenType::IDENT && !in_index) {
        auto access = std::make_unique<AstNode>(AstKind::FieldAccess, *token);
        access->setName(token->lexeme);
        access->addChild(std::move(base));
        return access;
      }
    }

    if (child->type() == NodeType::IndexList) {
      auto access = std::make_unique<AstNode>(AstKind::ArrayAccess);
      access->addChild(std::move(base));
      for (const auto& index_child : child->children()) {
        if (const auto* token = tokenOf(*index_child);
            token && token->type != TokenType::COMMA) {
          access->addChild(buildIndexElement(*index_child));
        }
      }
      return access;
    }
  }

  return base ? std::move(base) : makeError();
}

AstPtr SdtBuilder::buildIndexElement(const parser::ParseNode& node) const {
  const auto* token = tokenOf(node);
  if (!token) return makeError();

  if (token->type == TokenType::IDENT) {
    return make_var_ref(*token);
  }
  if (token->type == TokenType::INTCON || token->type == TokenType::CHARCON) {
    return make_literal(*token);
  }
  return makeError();
}

AstPtr SdtBuilder::buildFunctionCall(const parser::ParseNode& node) const {
  auto call = std::make_unique<AstNode>(AstKind::Call);

  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT && call->name().empty()) {
      call->setName(token->lexeme);
      continue;
    }

    if (child->type() == NodeType::ParameterList) {
      call->addChild(buildParameterList(*child));
    }
  }

  return call;
}

AstPtr SdtBuilder::buildParameterList(const parser::ParseNode& node) const {
  auto params = std::make_unique<AstNode>(AstKind::ParameterList);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::Expression) {
      params->addChild(buildExpression(*child));
    }
  }
  return params;
}

}  // namespace semantic
