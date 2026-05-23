#include <vector>

#include "ast/ast_node.hpp"
#include "ast/decl_nodes.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {

std::vector<Ptr<ast::AstNode>> SDTBuilder::buildDeclarations(
    const parser::ParseNode& node) {
  return std::vector<Ptr<ast::AstNode>>();
}

Ptr<ast::TypeSpecNode> SDTBuilder::buildTypeSpec(const parser::ParseNode& node) {
  return nullptr;
}

Ptr<ast::VarDeclNode> SDTBuilder::buildVarDecl(const parser::ParseNode& node) {
  return nullptr;
}

Ptr<ast::TypeDeclNode> SDTBuilder::buildTypeDecl(const parser::ParseNode& node) {
  return nullptr;
}

Ptr<ast::ProcDeclNode> SDTBuilder::buildProcDecl(const parser::ParseNode& node) {
  return nullptr;
}

Ptr<ast::FuncDeclNode> SDTBuilder::buildFuncDecl(const parser::ParseNode& node) {
  return nullptr;
}

std::vector<Ptr<ast::ParameterNode>> SDTBuilder::buildFormalParameters(
    const parser::ParseNode& node) {
  return std::vector<Ptr<ast::ParameterNode>>();
}

}  // namespace semantic
