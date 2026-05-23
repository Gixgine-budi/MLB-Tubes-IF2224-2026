#include "semantic/sdt_builder.hpp"

#include "ast/ast_node.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

Ptr<ast::AstNode> SDTBuilder::build(const parser::ParseNode &parse_node) {
  if (parse_node.type() == parser::NodeType::Program) {
    return buildProgram(parse_node);
  }
  return nullptr;
}

Ptr<ast::ProgramNode> SDTBuilder::buildProgram(const parser::ParseNode &node) {
  if (node.type() != parser::NodeType::Program) return nullptr;

  lexer::Token program_id;
  std::vector<lexer::Token> params;
  Ptr<ast::BlockNode> block = nullptr;

  const auto &children = node.children();
  if (children.empty()) return nullptr;

  if (children[0]->type() == parser::NodeType::ProgramHeader) {
    const auto &header_children = children[0]->children();
    for (const auto &hc : header_children) {
      if (hc->type() == parser::NodeType::TokenNode &&
          hc->token().has_value() &&
          hc->token()->type == lexer::TokenType::IDENT) {
        program_id = hc->token().value();
      }
    }
  }

  std::vector<Ptr<ast::AstNode>> declarations;
  Ptr<ast::CompoundStmtNode> comp_stmt = nullptr;

  for (size_t i = 1; i < children.size(); ++i) {
    if (children[i]->type() == parser::NodeType::DeclarationPart) {
      // Flatten returned declarations
      auto decls = buildDeclarations(*children[i]);
      for (auto &d : decls) {
        declarations.push_back(std::move(d));
      }
    } else if (children[i]->type() == parser::NodeType::CompoundStatement) {
      comp_stmt = buildCompoundStmt(*children[i]);
    }
  }

  block = std::make_unique<ast::BlockNode>(std::move(declarations),
                                           std::move(comp_stmt));

  return std::make_unique<ast::ProgramNode>(program_id, params,
                                            std::move(block));
}

Ptr<ast::BlockNode> SDTBuilder::buildBlock(const parser::ParseNode &node) {
  if (node.type() != parser::NodeType::Block) return nullptr;

  std::vector<Ptr<ast::AstNode>> declarations;
  Ptr<ast::CompoundStmtNode> comp_stmt = nullptr;

  for (const auto &child : node.children()) {
    if (child->type() == parser::NodeType::DeclarationPart) {
      auto decls = buildDeclarations(*child);
      for (auto &d : decls) {
        declarations.push_back(std::move(d));
      }
    } else if (child->type() == parser::NodeType::CompoundStatement) {
      comp_stmt = buildCompoundStmt(*child);
    }
  }

  return std::make_unique<ast::BlockNode>(std::move(declarations),
                                          std::move(comp_stmt));
}

}  // namespace semantic