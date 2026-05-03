#include <memory>

#include "parser.hpp"
#include "parser/parse_node.hpp"

namespace parser {

ParsePtr Parser::parseExpression() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  return node;
}

ParsePtr Parser::parseSimpleExpression() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  return node;
}

ParsePtr Parser::parseTerm() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  return node;
}

ParsePtr Parser::parseFactor() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  return node;
}

ParsePtr Parser::parseRelationalOperator() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  return node;
}

ParsePtr Parser::parseAdditiveOperator() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  return node;
}

ParsePtr Parser::parseMultiplicativeOperator() {
  auto node = std::make_unique<ParseNode>(NodeType::Expression);
  return node;
}

}  // namespace parser