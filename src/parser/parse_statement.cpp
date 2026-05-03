#include <memory>

#include "parser.hpp"
#include "parser/parse_node.hpp"

namespace parser {

ParsePtr Parser::parseCompoundStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseStatementList() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseVariable() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseComponentVariable() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseIndexList() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseAssignmentStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseIfStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseCaseStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseCaseBlock() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseWhileStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseRepeatStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseForStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseFunctionCall() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

ParsePtr Parser::parseParameterList() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  return node;
}

}  // namespace parser