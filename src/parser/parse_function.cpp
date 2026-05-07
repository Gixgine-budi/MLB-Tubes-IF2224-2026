#include <memory>

#include "diagnoser/diagnostic.hpp"
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
    diagnoser_.report(
        {diag::Phase::PARSER,
         diag::Level::ERROR,
         {current().line_num, current().col_num,
          static_cast<int>(std::max(size_t{1}, current().lexeme.size()))},
         "expected 'procedure' or 'function', found " + formatToken(current()),
         ""});
    sync({TokenType::SEMICOLON});
  }
  return node;
}

ParsePtr Parser::parseProcedureDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::ProcedureDeclaration);
  node->addChild(consume(TokenType::PROCEDURESY));
  node->addChild(consume(TokenType::IDENT, "as procedure name"));
  if (current().type == TokenType::LPARENT) {
    node->addChild(parseFormalParameterList());
  }
  node->addChild(consume(TokenType::SEMICOLON, "after procedure header"));
  node->addChild(parseBlock());
  node->addChild(consume(TokenType::SEMICOLON, "after procedure body"));
  return node;
}

ParsePtr Parser::parseFunctionDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::FunctionDeclaration);
  node->addChild(consume(TokenType::FUNCTIONSY));
  node->addChild(consume(TokenType::IDENT, "as function name"));
  if (current().type == TokenType::LPARENT) {
    node->addChild(parseFormalParameterList());
  }
  node->addChild(consume(TokenType::COLON, "after function parameters"));
  node->addChild(consume(TokenType::IDENT, "as return type"));
  node->addChild(consume(TokenType::SEMICOLON, "after function header"));
  node->addChild(parseBlock());
  node->addChild(consume(TokenType::SEMICOLON, "after function body"));
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
