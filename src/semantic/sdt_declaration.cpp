#include <memory>
#include <utility>
#include <vector>

#include "ast/ast_node.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "lexer/token.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"
#include "semantic/sdt_internal.hpp"

namespace semantic {

namespace {

Ptr<ast::AstNode> buildConstantValue(const parser::ParseNode& node) {
  for (const auto& child : node.children()) {
    if (auto tok = sdt::tokenOf(*child)) {
      switch (tok->type) {
        case lexer::TokenType::INTCON:
          return std::make_unique<ast::NumberNode>(*tok, false);
        case lexer::TokenType::REALCON:
          return std::make_unique<ast::NumberNode>(*tok, true);
        case lexer::TokenType::CHARCON:
        case lexer::TokenType::STRING:
          return std::make_unique<ast::StringNode>(*tok);
        case lexer::TokenType::IDENT:
          return std::make_unique<ast::IdentNode>(*tok);
        default:
          break;
      }
    }
    if (child->type() == parser::NodeType::Constant) {
      return buildConstantValue(*child);
    }
  }
  return nullptr;
}

}  // namespace

Ptr<ast::TypeSpecNode> SDTBuilder::buildTypeSpec(const parser::ParseNode& node) {
  const parser::ParseNode* target = &node;
  if (node.type() == parser::NodeType::Type && !node.children().empty()) {
    target = node.children().front().get();
  }

  switch (target->type()) {
    case parser::NodeType::TokenNode: {
      if (auto tok = sdt::tokenOf(*target)) {
        auto spec = std::make_unique<ast::TypeSpecNode>(ast::TypeSpecNode::Kind::Simple);
        spec->name = *tok;
        return spec;
      }
      break;
    }
    case parser::NodeType::Range: {
      auto spec = std::make_unique<ast::TypeSpecNode>(ast::TypeSpecNode::Kind::Subrange);
      const auto& kids = target->children();
      if (!kids.empty()) {
        spec->low = buildConstantValue(*kids[0]);
      }
      if (kids.size() >= 4) {
        spec->high = buildConstantValue(*kids[3]);
      }
      return spec;
    }
    case parser::NodeType::ArrayType: {
      auto spec = std::make_unique<ast::TypeSpecNode>(ast::TypeSpecNode::Kind::Array);
      const auto* index_node = sdt::findChild(*target, parser::NodeType::Range);
      if (index_node == nullptr) {
        for (const auto& child : target->children()) {
          if (auto tok = sdt::tokenOf(*child)) {
            if (tok->type == lexer::TokenType::IDENT) {
              spec->index_type = std::make_unique<ast::TypeSpecNode>(
                  ast::TypeSpecNode::Kind::Simple);
              spec->index_type->name = *tok;
              break;
            }
          }
        }
      } else {
        spec->index_type = buildTypeSpec(*index_node);
      }
      const auto* elem = sdt::findChild(*target, parser::NodeType::Type);
      if (elem != nullptr) {
        spec->element_type = buildTypeSpec(*elem);
      }
      return spec;
    }
    case parser::NodeType::RecordType: {
      auto spec = std::make_unique<ast::TypeSpecNode>(ast::TypeSpecNode::Kind::Record);
      const auto* fields = sdt::findChild(*target, parser::NodeType::FieldList);
      if (fields != nullptr) {
        for (const auto* part :
             sdt::childrenOfType(*fields, parser::NodeType::FieldPart)) {
          const auto* id_list =
              sdt::findChild(*part, parser::NodeType::IdentifierList);
          const auto* type_node = sdt::findChild(*part, parser::NodeType::Type);
          if (id_list != nullptr && type_node != nullptr) {
            spec->fields.emplace_back(sdt::collectIdentifiers(*id_list),
                                      buildTypeSpec(*type_node));
          }
        }
      }
      return spec;
    }
    case parser::NodeType::Enumerated: {
      auto spec = std::make_unique<ast::TypeSpecNode>(ast::TypeSpecNode::Kind::Enumerated);
      for (const auto& child : target->children()) {
        if (auto tok = sdt::tokenOf(*child)) {
          if (tok->type == lexer::TokenType::IDENT) {
            spec->enum_literals.push_back(*tok);
          }
        }
      }
      return spec;
    }
    default:
      break;
  }

  return std::make_unique<ast::TypeSpecNode>(ast::TypeSpecNode::Kind::Simple);
}

std::vector<Ptr<ast::AstNode>> SDTBuilder::buildDeclarations(
    const parser::ParseNode& node) {
  std::vector<Ptr<ast::AstNode>> decls;
  for (const auto& child : node.children()) {
    switch (child->type()) {
      case parser::NodeType::ConstDeclaration:
        break;
      case parser::NodeType::TypeDeclaration:
        if (auto type_decl = buildTypeDecl(*child)) {
          decls.push_back(std::move(type_decl));
        }
        break;
      case parser::NodeType::VarDeclaration:
        if (auto var_decl = buildVarDecl(*child)) {
          decls.push_back(std::move(var_decl));
        }
        break;
      case parser::NodeType::SubprogramDeclaration: {
        if (const auto* proc =
                sdt::findChild(*child, parser::NodeType::ProcedureDeclaration)) {
          if (auto p = buildProcDecl(*proc)) {
            decls.push_back(std::move(p));
          }
        } else if (const auto* func = sdt::findChild(
                       *child, parser::NodeType::FunctionDeclaration)) {
          if (auto f = buildFuncDecl(*func)) {
            decls.push_back(std::move(f));
          }
        }
        break;
      }
      default:
        break;
    }
  }
  return decls;
}

Ptr<ast::VarDeclNode> SDTBuilder::buildVarDecl(const parser::ParseNode& node) {
  std::vector<lexer::Token> identifiers;
  Ptr<ast::TypeSpecNode> type_spec;

  for (const auto& child : node.children()) {
    if (child->type() == parser::NodeType::IdentifierList) {
      auto ids = sdt::collectIdentifiers(*child);
      identifiers.insert(identifiers.end(), ids.begin(), ids.end());
    } else if (child->type() == parser::NodeType::Type) {
      type_spec = buildTypeSpec(*child);
    }
  }

  if (identifiers.empty() || type_spec == nullptr) {
    return nullptr;
  }

  Ptr<ast::AstNode> type_as_ast = std::move(type_spec);
  return std::make_unique<ast::VarDeclNode>(std::move(identifiers),
                                            std::move(type_as_ast));
}

Ptr<ast::TypeDeclNode> SDTBuilder::buildTypeDecl(const parser::ParseNode& node) {
  lexer::Token name{};
  Ptr<ast::TypeSpecNode> definition;

  for (const auto& child : node.children()) {
    if (auto tok = sdt::tokenOf(*child)) {
      if (tok->type == lexer::TokenType::IDENT && name.lexeme.empty()) {
        name = *tok;
      }
    } else if (child->type() == parser::NodeType::Type) {
      definition = buildTypeSpec(*child);
    }
  }

  if (name.lexeme.empty() || definition == nullptr) {
    return nullptr;
  }

  Ptr<ast::AstNode> def_as_ast = std::move(definition);
  return std::make_unique<ast::TypeDeclNode>(name, std::move(def_as_ast));
}

Ptr<ast::ProcDeclNode> SDTBuilder::buildProcDecl(const parser::ParseNode& node) {
  lexer::Token name{};
  std::vector<Ptr<ast::ParameterNode>> params;
  Ptr<ast::BlockNode> body;

  for (const auto& child : node.children()) {
    if (auto tok = sdt::tokenOf(*child)) {
      if (tok->type == lexer::TokenType::IDENT && name.lexeme.empty()) {
        name = *tok;
      }
    } else if (child->type() == parser::NodeType::FormalParameterList) {
      params = buildFormalParameters(*child);
    } else if (child->type() == parser::NodeType::Block) {
      body = buildBlock(*child);
    }
  }

  if (name.lexeme.empty() || body == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::ProcDeclNode>(name, std::move(params),
                                             std::move(body));
}

Ptr<ast::FuncDeclNode> SDTBuilder::buildFuncDecl(const parser::ParseNode& node) {
  lexer::Token name{};
  std::vector<Ptr<ast::ParameterNode>> params;
  Ptr<ast::TypeSpecNode> return_type;
  Ptr<ast::BlockNode> body;

  for (const auto& child : node.children()) {
    if (auto tok = sdt::tokenOf(*child)) {
      if (tok->type == lexer::TokenType::IDENT) {
        if (name.lexeme.empty()) {
          name = *tok;
        } else if (return_type == nullptr) {
          return_type = std::make_unique<ast::TypeSpecNode>(
              ast::TypeSpecNode::Kind::Simple);
          return_type->name = *tok;
        }
      }
    } else if (child->type() == parser::NodeType::FormalParameterList) {
      params = buildFormalParameters(*child);
    } else if (child->type() == parser::NodeType::Block) {
      body = buildBlock(*child);
    }
  }

  if (name.lexeme.empty() || return_type == nullptr || body == nullptr) {
    return nullptr;
  }

  Ptr<ast::AstNode> ret_as_ast = std::move(return_type);
  return std::make_unique<ast::FuncDeclNode>(
      name, std::move(params), std::move(ret_as_ast), std::move(body));
}

std::vector<Ptr<ast::ParameterNode>> SDTBuilder::buildFormalParameters(
    const parser::ParseNode& node) {
  std::vector<Ptr<ast::ParameterNode>> params;
  for (const auto& child : node.children()) {
    if (child->type() == parser::NodeType::ParameterGroup) {
      std::vector<lexer::Token> ids;
      Ptr<ast::TypeSpecNode> type_spec;
      for (const auto& part : child->children()) {
        if (part->type() == parser::NodeType::IdentifierList) {
          auto collected = sdt::collectIdentifiers(*part);
          ids.insert(ids.end(), collected.begin(), collected.end());
        } else if (part->type() == parser::NodeType::Type ||
                   part->type() == parser::NodeType::ArrayType) {
          type_spec = buildTypeSpec(*part);
        }
      }
      if (!ids.empty() && type_spec != nullptr) {
        Ptr<ast::AstNode> param_type = std::move(type_spec);
        params.push_back(std::make_unique<ast::ParameterNode>(
            std::move(ids), std::move(param_type), false));
      }
    }
  }
  return params;
}

}  // namespace semantic
