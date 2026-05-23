#include <vector>

#include "ast/ast_node.hpp"
#include "ast/decl_nodes.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {

// Placeholder type build - returning nullptr for now until Types are mapped
Ptr<ast::AstNode> buildTypeHelper(const parser::ParseNode &node) {
  return nullptr;
}

std::vector<Ptr<ast::AstNode>> SDTBuilder::buildDeclarations(
    const parser::ParseNode &node) {
  std::vector<Ptr<ast::AstNode>> decls;
  if (node.type() != parser::NodeType::DeclarationPart) return decls;

  for (const auto &child : node.children()) {
    if (child->type() == parser::NodeType::VarDeclaration) {
      size_t child_count = child->children().size();
      for (size_t i = 1; i + 2 < child_count; i += 4) {
        const auto &id_list_node = child->children()[i];
        std::vector<lexer::Token> ids;
        for (const auto &id_tok_node : id_list_node->children()) {
          if (id_tok_node->type() == parser::NodeType::TokenNode &&
              id_tok_node->token().has_value() &&
              id_tok_node->token()->type == lexer::TokenType::IDENT) {
            ids.push_back(id_tok_node->token().value());
          }
        }
        const auto &type_node = child->children()[i + 2];
        decls.push_back(std::make_unique<ast::VarDeclNode>(
            std::move(ids), buildTypeHelper(*type_node)));
      }
    } else if (child->type() == parser::NodeType::TypeDeclaration) {
      size_t child_count = child->children().size();
      for (size_t i = 1; i + 2 < child_count; i += 4) {
        const auto &id_node = child->children()[i];
        if (!id_node->token().has_value()) continue;
        lexer::Token id_tok = id_node->token().value();
        const auto &type_node = child->children()[i + 2];
        decls.push_back(std::make_unique<ast::TypeDeclNode>(
            id_tok, buildTypeHelper(*type_node)));
      }
    } else if (child->type() == parser::NodeType::SubprogramDeclaration) {
      if (!child->children().empty()) {
        auto sub_node = child->children()[0].get();
        if (sub_node->type() == parser::NodeType::ProcedureDeclaration) {
          decls.push_back(buildProcDecl(*sub_node));
        } else if (sub_node->type() == parser::NodeType::FunctionDeclaration) {
          decls.push_back(buildFuncDecl(*sub_node));
        }
      }
    }
  }
  return decls;
}

Ptr<ast::VarDeclNode> SDTBuilder::buildVarDecl(const parser::ParseNode &node) {
  return nullptr;  // Handled directly in buildDeclarations loop
}

Ptr<ast::TypeDeclNode> SDTBuilder::buildTypeDecl(
    const parser::ParseNode &node) {
  return nullptr;  // Handled directly in buildDeclarations loop
}

Ptr<ast::ProcDeclNode> SDTBuilder::buildProcDecl(
    const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  lexer::Token id_tok;
  std::vector<Ptr<ast::ParameterNode>> params;
  Ptr<ast::BlockNode> block;

  size_t idx = 1;
  if (idx < node.children().size() &&
      node.children()[idx]->type() == parser::NodeType::TokenNode) {
    id_tok = node.children()[idx]->token().value();
    idx++;
  }

  if (idx < node.children().size() &&
      node.children()[idx]->type() == parser::NodeType::FormalParameterList) {
    params = buildFormalParameters(*node.children()[idx]);
    idx++;
  }

  idx++;  // Skip SEMICOLON

  if (idx < node.children().size() &&
      node.children()[idx]->type() == parser::NodeType::Block) {
    block = buildBlock(*node.children()[idx]);
  }

  return std::make_unique<ast::ProcDeclNode>(id_tok, std::move(params),
                                             std::move(block));
}

Ptr<ast::FuncDeclNode> SDTBuilder::buildFuncDecl(
    const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  lexer::Token id_tok;
  std::vector<Ptr<ast::ParameterNode>> params;
  Ptr<ast::AstNode> ret_type = nullptr;
  Ptr<ast::BlockNode> block;

  size_t idx = 1;
  if (idx < node.children().size() &&
      node.children()[idx]->type() == parser::NodeType::TokenNode) {
    id_tok = node.children()[idx]->token().value();
    idx++;
  }

  if (idx < node.children().size() &&
      node.children()[idx]->type() == parser::NodeType::FormalParameterList) {
    params = buildFormalParameters(*node.children()[idx]);
    idx++;
  }

  idx++;  // Skip COLON

  if (idx < node.children().size() &&
      node.children()[idx]->type() == parser::NodeType::TokenNode) {
    idx++;  // Read return type
  }

  idx++;  // Skip SEMICOLON

  if (idx < node.children().size() &&
      node.children()[idx]->type() == parser::NodeType::Block) {
    block = buildBlock(*node.children()[idx]);
  }

  return std::make_unique<ast::FuncDeclNode>(
      id_tok, std::move(params), std::move(ret_type), std::move(block));
}

std::vector<Ptr<ast::ParameterNode>> SDTBuilder::buildFormalParameters(
    const parser::ParseNode &node) {
  std::vector<Ptr<ast::ParameterNode>> params;
  for (const auto &child : node.children()) {
    if (child->type() == parser::NodeType::ParameterGroup) {
      bool is_var = false;
      size_t c_idx = 0;
      if (child->children()[0]->type() == parser::NodeType::TokenNode &&
          child->children()[0]->token()->type == lexer::TokenType::VARSY) {
        is_var = true;
        c_idx++;
      }

      std::vector<lexer::Token> ids;
      if (c_idx < child->children().size() &&
          child->children()[c_idx]->type() ==
              parser::NodeType::IdentifierList) {
        const auto &id_list_node = child->children()[c_idx];
        for (const auto &id_tok_node : id_list_node->children()) {
          if (id_tok_node->type() == parser::NodeType::TokenNode &&
              id_tok_node->token().has_value() &&
              id_tok_node->token()->type == lexer::TokenType::IDENT) {
            ids.push_back(id_tok_node->token().value());
          }
        }
        c_idx++;
      }

      c_idx++;  // Skip ':'

      Ptr<ast::AstNode> type_node = nullptr;
      if (c_idx < child->children().size()) {
        type_node = buildTypeHelper(*child->children()[c_idx]);
      }

      params.push_back(std::make_unique<ast::ParameterNode>(
          std::move(ids), std::move(type_node), is_var));
    }
  }
  return params;
}

}  // namespace semantic