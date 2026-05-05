#include <memory>

#include "arion_exceptions.hpp"
#include "lexer/token.hpp"
#include "parser.hpp"
#include "parser/parse_node.hpp"

namespace parser {

using lexer::TokenType;

ParsePtr Parser::parseSubprogramDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::SubprogramDeclaration);
  if (current().type == TokenType::PROCEDURESY) {
    node->addChild(parseProcedureDeclaration());
  } else if (current().type == TokenType::FUNCTIONSY) {
    node->addChild(parseFunctionDeclaration());
  } else {
    throw UnexpectedTokenException(filename_, TokenType::PROCEDURESY, current());
  }
  return node;
}

ParsePtr Parser::parseProcedureDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::ProcedureDeclaration);
  node->addChild(consume(TokenType::PROCEDURESY));
  node->addChild(consume(TokenType::IDENT));
  if (current().type == TokenType::LPARENT) {
    node->addChild(parseFormalParameterList());
  }
  node->addChild(consume(TokenType::SEMICOLON));
  node->addChild(parseBlock());
  node->addChild(consume(TokenType::SEMICOLON));
  return node;
}

ParsePtr Parser::parseFunctionDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::FunctionDeclaration);
  node->addChild(consume(TokenType::FUNCTIONSY));
  node->addChild(consume(TokenType::IDENT));
  if (current().type == TokenType::LPARENT) {
    node->addChild(parseFormalParameterList());
  }
  node->addChild(consume(TokenType::COLON));
  node->addChild(consume(TokenType::IDENT));
  node->addChild(consume(TokenType::SEMICOLON));
  node->addChild(parseBlock());
  node->addChild(consume(TokenType::SEMICOLON));
  return node;
}

ParsePtr Parser::parseBlock() {
  auto node = std::make_unique<ParseNode>(NodeType::Block);
  node->addChild(parseDeclarationPart());
  node->addChild(parseCompoundStatement());
  return node;
}

ParsePtr Parser::parseFormalParameterList() {
  auto node = std::make_unique<ParseNode>(NodeType::FormalParameterList);
  node->addChild(consume(TokenType::LPARENT));
  node->addChild(parseParameterGroup());
  while (current().type == TokenType::SEMICOLON) {
    node->addChild(consume(TokenType::SEMICOLON));
    node->addChild(parseParameterGroup());
  }
  node->addChild(consume(TokenType::RPARENT));
  return node;
}

ParsePtr Parser::parseParameterGroup() {
  auto node = std::make_unique<ParseNode>(NodeType::ParameterGroup);
  node->addChild(parseIdentifierList());
  node->addChild(consume(TokenType::COLON));
  if (current().type == TokenType::ARRAYSY) {
    node->addChild(parseArrayType());
  } else {
    node->addChild(consume(TokenType::IDENT));
  }
  return node;
}

}  // namespace parser
