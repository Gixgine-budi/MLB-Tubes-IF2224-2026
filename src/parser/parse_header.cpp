#include <memory>
#include <stdexcept>

#include "lexer/token.hpp"
#include "parser.hpp"
#include "parser/parse_node.hpp"

namespace parser {

using lexer::TokenType;

namespace {

bool isSignedConstantBody(TokenType type) {
  return type == TokenType::IDENT || type == TokenType::INTCON ||
         type == TokenType::REALCON;
}

}  // namespace

void Parser::parseProgram() {
  program_.addChild(parseProgramHeader());
  program_.addChild(parseDeclarationPart());
  program_.addChild(parseCompoundStatement());
  program_.addChild(consume(TokenType::PERIOD));

  if (!is_done())
    throw std::runtime_error("TODO: EXCEPTION masih ada sisa after period");
}

ParsePtr Parser::parseProgramHeader() {
  auto node = std::make_unique<ParseNode>(NodeType::ProgramHeader);
  node->addChild(consume(TokenType::PROGRAMSY));
  node->addChild(consume(TokenType::IDENT));
  node->addChild(consume(TokenType::SEMICOLON));

  return node;
}

ParsePtr Parser::parseDeclarationPart() {
  auto node = std::make_unique<ParseNode>(NodeType::DeclarationPart);

  while (!is_done() && current().type == TokenType::CONSTSY) {
    node->addChild(parseConstDeclaration());
  }

  while (!is_done() && current().type == TokenType::TYPESY) {
    node->addChild(parseTypeDeclaration());
  }

  while (!is_done() && current().type == TokenType::VARSY) {
    node->addChild(parseVarDeclaration());
  }

  while (!is_done() && (current().type == TokenType::PROCEDURESY ||
                        current().type == TokenType::FUNCTIONSY)) {
    const auto previous_position = position_;
    node->addChild(parseSubprogramDeclaration());
    if (position_ == previous_position) {
      throw std::runtime_error(
          "internal parser error: subprogram parser made no progress");
    }
  }

  return node;
}

ParsePtr Parser::parseConstDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::ConstDeclaration);
  node->addChild(consume(TokenType::CONSTSY));

  do {
    node->addChild(consume(TokenType::IDENT));
    node->addChild(consume(TokenType::EQL));
    node->addChild(parseConstant());
    node->addChild(consume(TokenType::SEMICOLON));
  } while (!is_done() && current().type == TokenType::IDENT);

  return node;
}

ParsePtr Parser::parseConstant() {
  auto node = std::make_unique<ParseNode>(NodeType::Constant);

  switch (current().type) {
    case TokenType::CHARCON:
      node->addChild(consume(TokenType::CHARCON));
      break;
    case TokenType::STRING:
      node->addChild(consume(TokenType::STRING));
      break;
    case TokenType::PLUS:
    case TokenType::MINUS: {
      const auto sign = current().type;
      node->addChild(consume(sign));
      if (!isSignedConstantBody(current().type)) {
        node->addChild(consume(TokenType::IDENT));
      } else {
        const auto body = current().type;
        node->addChild(consume(body));
      }
      break;
    }
    case TokenType::IDENT:
    case TokenType::INTCON:
    case TokenType::REALCON: {
      const auto type = current().type;
      node->addChild(consume(type));
      break;
    }
    default:
      node->addChild(consume(TokenType::IDENT));
      break;
  }

  return node;
}

ParsePtr Parser::parseTypeDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::TypeDeclaration);
  node->addChild(consume(TokenType::TYPESY));

  do {
    node->addChild(consume(TokenType::IDENT));
    node->addChild(consume(TokenType::EQL));
    node->addChild(parseType());
    node->addChild(consume(TokenType::SEMICOLON));
  } while (!is_done() && current().type == TokenType::IDENT);

  return node;
}

ParsePtr Parser::parseVarDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::VarDeclaration);
  node->addChild(consume(TokenType::VARSY));

  do {
    node->addChild(parseIdentifierList());
    node->addChild(consume(TokenType::COLON));
    node->addChild(parseType());
    node->addChild(consume(TokenType::SEMICOLON));
  } while (!is_done() && current().type == TokenType::IDENT);

  return node;
}

ParsePtr Parser::parseIdentifierList() {
  auto node = std::make_unique<ParseNode>(NodeType::IdentifierList);
  node->addChild(consume(TokenType::IDENT));

  while (!is_done() && current().type == TokenType::COMMA) {
    node->addChild(consume(TokenType::COMMA));
    node->addChild(consume(TokenType::IDENT));
  }

  return node;
}

ParsePtr Parser::parseType() {
  auto node = std::make_unique<ParseNode>(NodeType::Type);

  const auto startsRange = [&]() {
    if (current().type == TokenType::IDENT || current().type == TokenType::INTCON ||
        current().type == TokenType::REALCON || current().type == TokenType::CHARCON ||
        current().type == TokenType::STRING) {
      return position_ + 2 < tokens_.size() &&
             tokens_.at(position_ + 1).type == TokenType::PERIOD &&
             tokens_.at(position_ + 2).type == TokenType::PERIOD;
    }

    if (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
      return position_ + 3 < tokens_.size() &&
             isSignedConstantBody(tokens_.at(position_ + 1).type) &&
             tokens_.at(position_ + 2).type == TokenType::PERIOD &&
             tokens_.at(position_ + 3).type == TokenType::PERIOD;
    }

    return false;
  };

  switch (current().type) {
    case TokenType::ARRAYSY:
      node->addChild(parseArrayType());
      break;
    case TokenType::LPARENT:
      node->addChild(parseEnumerated());
      break;
    case TokenType::RECORDSY:
      node->addChild(parseRecordType());
      break;
    case TokenType::IDENT:
      if (startsRange()) {
        node->addChild(parseRange());
      } else {
        node->addChild(consume(TokenType::IDENT));
      }
      break;
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::INTCON:
    case TokenType::REALCON:
    case TokenType::CHARCON:
    case TokenType::STRING:
      node->addChild(parseRange());
      break;
    default:
      node->addChild(consume(TokenType::IDENT));
      break;
  }

  return node;
}

ParsePtr Parser::parseArrayType() {
  auto node = std::make_unique<ParseNode>(NodeType::ArrayType);
  node->addChild(consume(TokenType::ARRAYSY));
  node->addChild(consume(TokenType::LBRACK));

  if (current().type == TokenType::IDENT &&
      !(position_ + 2 < tokens_.size() &&
        tokens_.at(position_ + 1).type == TokenType::PERIOD &&
        tokens_.at(position_ + 2).type == TokenType::PERIOD)) {
    node->addChild(consume(TokenType::IDENT));
  } else {
    node->addChild(parseRange());
  }

  node->addChild(consume(TokenType::RBRACK));
  node->addChild(consume(TokenType::OFSY));
  node->addChild(parseType());

  return node;
}

ParsePtr Parser::parseRange() {
  auto node = std::make_unique<ParseNode>(NodeType::Range);
  node->addChild(parseConstant());
  node->addChild(consume(TokenType::PERIOD));
  node->addChild(consume(TokenType::PERIOD));
  node->addChild(parseConstant());

  return node;
}

ParsePtr Parser::parseEnumerated() {
  auto node = std::make_unique<ParseNode>(NodeType::Enumerated);
  node->addChild(consume(TokenType::LPARENT));
  node->addChild(consume(TokenType::IDENT));

  while (!is_done() && current().type == TokenType::COMMA) {
    node->addChild(consume(TokenType::COMMA));
    node->addChild(consume(TokenType::IDENT));
  }

  node->addChild(consume(TokenType::RPARENT));

  return node;
}

ParsePtr Parser::parseRecordType() {
  auto node = std::make_unique<ParseNode>(NodeType::RecordType);
  node->addChild(consume(TokenType::RECORDSY));
  node->addChild(parseFieldList());
  node->addChild(consume(TokenType::ENDSY));

  return node;
}

ParsePtr Parser::parseFieldList() {
  auto node = std::make_unique<ParseNode>(NodeType::FieldList);
  node->addChild(parseFieldPart());

  while (!is_done() && current().type == TokenType::SEMICOLON) {
    node->addChild(consume(TokenType::SEMICOLON));

    if (!is_done() && current().type == TokenType::IDENT) {
      node->addChild(parseFieldPart());
    }
  }

  return node;
}

ParsePtr Parser::parseFieldPart() {
  auto node = std::make_unique<ParseNode>(NodeType::FieldPart);
  node->addChild(parseIdentifierList());
  node->addChild(consume(TokenType::COLON));
  node->addChild(parseType());

  return node;
}

}  // namespace parser
