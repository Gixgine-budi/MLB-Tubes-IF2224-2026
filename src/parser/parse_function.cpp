#include <memory>

#include "parser.hpp"
#include "parser/parse_node.hpp"

namespace parser {

ParsePtr Parser::parseSubprogramDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::SubprogramDeclaration);
  return node;
}

ParsePtr Parser::parseProcedureDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::SubprogramDeclaration);
  return node;
}

ParsePtr Parser::parseFunctionDeclaration() {
  auto node = std::make_unique<ParseNode>(NodeType::SubprogramDeclaration);
  return node;
}

ParsePtr Parser::parseBlock() {
  auto node = std::make_unique<ParseNode>(NodeType::SubprogramDeclaration);
  return node;
}

ParsePtr Parser::parseFormalParameterList() {
  auto node = std::make_unique<ParseNode>(NodeType::SubprogramDeclaration);
  return node;
}

ParsePtr Parser::parseParameterGroup() {
  auto node = std::make_unique<ParseNode>(NodeType::SubprogramDeclaration);
  return node;
}

}  // namespace parser