#include <vector>

#include "ast/ast_node.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/type_nodes.hpp"
#include "lexer/token.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {

namespace {

Ptr<ast::ExprNode> buildConstantExpr(const parser::ParseNode &node) {
  if (node.type() != parser::NodeType::Constant || node.children().empty()) {
    return nullptr;
  }

  const auto &children = node.children();
  const auto &first = children[0];
  if (first->type() != parser::NodeType::TokenNode || !first->token()) {
    return nullptr;
  }

  const lexer::Token first_tok = first->token().value();
  if ((first_tok.type == lexer::TokenType::PLUS ||
       first_tok.type == lexer::TokenType::MINUS) &&
      children.size() >= 2 &&
      children[1]->type() == parser::NodeType::TokenNode &&
      children[1]->token()) {
    const lexer::Token body_tok = children[1]->token().value();
    Ptr<ast::ExprNode> body_expr;
    if (body_tok.type == lexer::TokenType::INTCON ||
        body_tok.type == lexer::TokenType::REALCON) {
      body_expr = std::make_unique<ast::NumberNode>(
          body_tok, body_tok.type == lexer::TokenType::REALCON);
    } else {
      body_expr = std::make_unique<ast::IdentNode>(body_tok);
    }
    return std::make_unique<ast::UnaryOpNode>(first_tok, std::move(body_expr));
  }

  if (first_tok.type == lexer::TokenType::INTCON ||
      first_tok.type == lexer::TokenType::REALCON) {
    return std::make_unique<ast::NumberNode>(
        first_tok, first_tok.type == lexer::TokenType::REALCON);
  }
  if (first_tok.type == lexer::TokenType::CHARCON ||
      first_tok.type == lexer::TokenType::STRING) {
    return std::make_unique<ast::StringNode>(first_tok);
  }
  return std::make_unique<ast::IdentNode>(first_tok);
}

}  // namespace

Ptr<ast::TypeSpecNode> SDTBuilder::buildTypeSpec(
    const parser::ParseNode &node) {
  using parser::NodeType;

  switch (node.type()) {
    case NodeType::Type:
      if (node.children().empty()) return nullptr;
      return buildTypeSpec(*node.children()[0]);

    case NodeType::TokenNode:
      if (node.token().has_value() &&
          node.token()->type == lexer::TokenType::IDENT) {
        return std::make_unique<ast::SimpleTypeSpecNode>(node.token().value());
      }
      return nullptr;

    case NodeType::Range: {
      if (node.children().size() < 4) return nullptr;
      std::unique_ptr<ast::AstNode> low =
          buildConstantExpr(*node.children()[0]);
      std::unique_ptr<ast::AstNode> high =
          buildConstantExpr(*node.children()[3]);
      return std::make_unique<ast::SubrangeTypeSpecNode>(std::move(low),
                                                         std::move(high));
    }

    case NodeType::ArrayType: {
      if (node.children().size() < 6) return nullptr;

      Ptr<ast::TypeSpecNode> index_type;
      const auto &index_node = *node.children()[2];
      if (index_node.type() == NodeType::TokenNode && index_node.token() &&
          index_node.token()->type == lexer::TokenType::IDENT) {
        index_type = std::make_unique<ast::SimpleTypeSpecNode>(
            index_node.token().value());
      } else {
        index_type = buildTypeSpec(index_node);
      }

      auto element_type = buildTypeSpec(*node.children()[5]);
      return std::make_unique<ast::ArrayTypeSpecNode>(std::move(index_type),
                                                      std::move(element_type));
    }

    case NodeType::Enumerated: {
      std::vector<lexer::Token> literals;
      literals.reserve(node.children().size());
      for (const auto &child : node.children()) {
        if (child->type() == NodeType::TokenNode && child->token() &&
            child->token()->type == lexer::TokenType::IDENT) {
          literals.push_back(child->token().value());
        }
      }
      return std::make_unique<ast::EnumTypeSpecNode>(std::move(literals));
    }

    case NodeType::RecordType: {
      std::vector<std::pair<std::vector<lexer::Token>,
                            std::unique_ptr<ast::TypeSpecNode>>>
          fields;

      if (node.children().size() >= 2 &&
          node.children()[1]->type() == NodeType::FieldList) {
        const auto &field_list = node.children()[1];
        for (const auto &entry : field_list->children()) {
          if (entry->type() != NodeType::FieldPart) continue;

          std::vector<lexer::Token> ids;
          if (!entry->children().empty() &&
              entry->children()[0]->type() == NodeType::IdentifierList) {
            for (const auto &id_node : entry->children()[0]->children()) {
              if (id_node->type() == NodeType::TokenNode && id_node->token() &&
                  id_node->token()->type == lexer::TokenType::IDENT) {
                ids.push_back(id_node->token().value());
              }
            }
          }

          std::unique_ptr<ast::TypeSpecNode> field_type = nullptr;
          if (entry->children().size() >= 3) {
            field_type = buildTypeSpec(*entry->children()[2]);
          }

          fields.emplace_back(std::move(ids), std::move(field_type));
        }
      }

      return std::make_unique<ast::RecordTypeSpecNode>(std::move(fields));
    }

    default:
      return nullptr;
  }
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
            std::move(ids), buildTypeSpec(*type_node)));
      }
    } else if (child->type() == parser::NodeType::TypeDeclaration) {
      size_t child_count = child->children().size();
      for (size_t i = 1; i + 2 < child_count; i += 4) {
        const auto &id_node = child->children()[i];
        if (!id_node->token().has_value()) continue;
        lexer::Token id_tok = id_node->token().value();
        const auto &type_node = child->children()[i + 2];
        decls.push_back(std::make_unique<ast::TypeDeclNode>(
            id_tok, buildTypeSpec(*type_node)));
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
  Ptr<ast::TypeSpecNode> ret_type = nullptr;
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
    const auto &ret_tok_node = node.children()[idx];
    if (ret_tok_node->token().has_value() &&
        ret_tok_node->token()->type == lexer::TokenType::IDENT) {
      ret_type = std::make_unique<ast::SimpleTypeSpecNode>(
          ret_tok_node->token().value());
    }
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

      Ptr<ast::TypeSpecNode> type_node = nullptr;
      if (c_idx < child->children().size()) {
        type_node = buildTypeSpec(*child->children()[c_idx]);
      }

      params.push_back(std::make_unique<ast::ParameterNode>(
          std::move(ids), std::move(type_node), is_var));
    }
  }
  return params;
}

}  // namespace semantic
