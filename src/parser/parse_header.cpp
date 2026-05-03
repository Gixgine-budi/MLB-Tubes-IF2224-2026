#include <memory>
#include <stdexcept>

#include "lexer/token.hpp"
#include "parser.hpp"
#include "parser/parse_node.hpp"

namespace parser {

using lexer::TokenType;

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
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseConstDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseConstant() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseTypeDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseVarDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseIdentifierList() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseType() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseArrayType() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseRange() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseEnumerated() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseRecordType() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseFieldList() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

ParsePtr Parser::parseFieldPart() {
  auto node = std::make_unique<ParseNode>(NodeType::Program);
  return node;
}

}  // namespace parser
