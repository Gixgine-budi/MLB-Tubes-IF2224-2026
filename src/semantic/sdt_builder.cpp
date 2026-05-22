#include "semantic/sdt_builder.hpp"

#include "ast/ast_node.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

Ptr<ast::AstNode> SDTBuilder::build(const parser::ParseNode &parse_node) {
  return buildProgram(parse_node);
}

Ptr<ast::ProgramNode> SDTBuilder::buildProgram(const parser::ParseNode &node) {
  return nullptr;
}

Ptr<ast::BlockNode> SDTBuilder::buildBlock(const parser::ParseNode &node) {
  return nullptr;
}

}  // namespace semantic