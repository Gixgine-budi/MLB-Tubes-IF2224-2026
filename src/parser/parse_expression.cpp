#include <memory>
#include "parser/parser.hpp"
#include "parser/parse_node.hpp"
#include "lexer/token.hpp"

namespace parser {

ParsePtr Parser::parseExpression() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  node->addChild(parseSimpleExpression());

  auto t = current().type;
  if (t == lexer::TokenType::EQL || t == lexer::TokenType::NEQ ||
      t == lexer::TokenType::GTR || t == lexer::TokenType::GEQ ||
      t == lexer::TokenType::LSS || t == lexer::TokenType::LEQ) {
    node->addChild(parseRelationalOperator());
    node->addChild(parseSimpleExpression());
  }
  return node;
}

ParsePtr Parser::parseSimpleExpression() {
  auto node = std::make_unique<ParseNode>(NodeType::SimpleExpression);
  
  auto t = current().type;
  if (t == lexer::TokenType::PLUS || t == lexer::TokenType::MINUS) {
    node->addChild(consume(t));
  }

  node->addChild(parseTerm());

  t = current().type;
  while (t == lexer::TokenType::PLUS || t == lexer::TokenType::MINUS || t == lexer::TokenType::ORSY) {
    node->addChild(parseAdditiveOperator());
    node->addChild(parseTerm());
    t = current().type; 
  }
  return node;
}

ParsePtr Parser::parseTerm() {
  auto node = std::make_unique<ParseNode>(NodeType::Term);
  node->addChild(parseFactor());

  auto t = current().type;
  while (t == lexer::TokenType::TIMES || t == lexer::TokenType::RDIV ||
         t == lexer::TokenType::IDIV || t == lexer::TokenType::IMOD ||
         t == lexer::TokenType::ANDSY) {
    node->addChild(parseMultiplicativeOperator());
    node->addChild(parseFactor());
    t = current().type; 
  }
  return node;
}

ParsePtr Parser::parseFactor() {
  auto node = std::make_unique<ParseNode>(NodeType::Factor);
  auto t = current().type;

  if (t == lexer::TokenType::INTCON || t == lexer::TokenType::REALCON ||
      t == lexer::TokenType::CHARCON || t == lexer::TokenType::STRING) {
    node->addChild(consume(t));
  } else if (t == lexer::TokenType::LPARENT) {
    node->addChild(consume(lexer::TokenType::LPARENT));
    node->addChild(parseExpression());
    node->addChild(consume(lexer::TokenType::RPARENT));
  } else if (t == lexer::TokenType::NOTSY) {
    node->addChild(consume(lexer::TokenType::NOTSY));
    node->addChild(parseFactor());
  } else if (t == lexer::TokenType::IDENT) {
    if (peek(1).type == lexer::TokenType::LPARENT) {
      node->addChild(parseFunctionCall());
    } else {
      node->addChild(parseVariable());
    }
  } else {
    consume(lexer::TokenType::IDENT);
  }
  return node;
}

ParsePtr Parser::parseRelationalOperator() {
  auto node = std::make_unique<ParseNode>(NodeType::RelationalOperator);
  auto t = current().type;
  if (t == lexer::TokenType::EQL || t == lexer::TokenType::NEQ ||
      t == lexer::TokenType::GTR || t == lexer::TokenType::GEQ ||
      t == lexer::TokenType::LSS || t == lexer::TokenType::LEQ) {
    node->addChild(consume(t));
  } else {
    consume(lexer::TokenType::EQL); 
  }
  return node;
}

ParsePtr Parser::parseAdditiveOperator() {
  auto node = std::make_unique<ParseNode>(NodeType::AdditiveOperator);
  auto t = current().type;
  if (t == lexer::TokenType::PLUS || t == lexer::TokenType::MINUS || t == lexer::TokenType::ORSY) {
    node->addChild(consume(t));
  } else {
    consume(lexer::TokenType::PLUS); 
  }
  return node;
}

ParsePtr Parser::parseMultiplicativeOperator() {
  auto node = std::make_unique<ParseNode>(NodeType::MultiplicativeOperator);
  auto t = current().type;
  if (t == lexer::TokenType::TIMES || t == lexer::TokenType::RDIV ||
      t == lexer::TokenType::IDIV || t == lexer::TokenType::IMOD ||
      t == lexer::TokenType::ANDSY) {
    node->addChild(consume(t));
  } else {
    consume(lexer::TokenType::TIMES);
  }
  return node;
}

}  