#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {
Ptr<ast::ExprNode> SDTBuilder::buildExpression(const parser::ParseNode &node) {
  return nullptr;
}

Ptr<ast::ExprNode> SDTBuilder::buildSimpleExpression(
    const parser::ParseNode &node) {
  return nullptr;
}

Ptr<ast::ExprNode> SDTBuilder::buildTerm(const parser::ParseNode &node) {
  return nullptr;
}

Ptr<ast::ExprNode> SDTBuilder::buildFactor(const parser::ParseNode &node) {
  return nullptr;
}

}  // namespace semantic