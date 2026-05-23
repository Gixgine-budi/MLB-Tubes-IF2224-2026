#include "semantic/sdt_builder.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

namespace {

using lexer::TokenType;
using parser::NodeType;

bool is_ignored_declaration_token(TokenType type) {
  return type == TokenType::CONSTSY || type == TokenType::TYPESY ||
         type == TokenType::VARSY || type == TokenType::EQL ||
         type == TokenType::COLON || type == TokenType::SEMICOLON ||
         type == TokenType::COMMA || type == TokenType::ARRAYSY ||
         type == TokenType::LBRACK || type == TokenType::RBRACK ||
         type == TokenType::OFSY || type == TokenType::RECORDSY ||
         type == TokenType::ENDSY || type == TokenType::LPARENT ||
         type == TokenType::RPARENT || type == TokenType::PERIOD;
}

AstPtr named_type_from_token(const lexer::Token& token) {
  auto node = std::make_unique<AstNode>(AstKind::NamedType, token);
  node->setName(token.lexeme);
  return node;
}

}  // namespace

AstPtr SdtBuilder::buildDeclarationPart(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::ConstDeclaration ||
        child->type() == NodeType::TypeDeclaration ||
        child->type() == NodeType::VarDeclaration) {
      ast->addChild(build(*child));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildConstDeclaration(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto& children = node.children();

  for (std::size_t i = 0; i < children.size(); ++i) {
    const auto* token = tokenOf(*children[i]);
    if (!token || token->type != TokenType::IDENT) continue;

    auto decl = std::make_unique<AstNode>(AstKind::ConstDecl, *token);
    decl->setName(token->lexeme);

    for (std::size_t j = i + 1; j < children.size(); ++j) {
      if (children[j]->type() == NodeType::Constant) {
        decl->addChild(buildConstant(*children[j]));
        i = j;
        break;
      }
      if (const auto* next_token = tokenOf(*children[j]);
          next_token && next_token->type == TokenType::IDENT) {
        break;
      }
    }

    ast->addChild(std::move(decl));
  }

  return ast;
}

AstPtr SdtBuilder::buildTypeDeclaration(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto& children = node.children();

  for (std::size_t i = 0; i < children.size(); ++i) {
    const auto* token = tokenOf(*children[i]);
    if (!token || token->type != TokenType::IDENT) continue;

    auto decl = std::make_unique<AstNode>(AstKind::TypeDecl, *token);
    decl->setName(token->lexeme);

    for (std::size_t j = i + 1; j < children.size(); ++j) {
      if (children[j]->type() == NodeType::Type) {
        decl->addChild(buildType(*children[j]));
        i = j;
        break;
      }
      if (const auto* next_token = tokenOf(*children[j]);
          next_token && next_token->type == TokenType::IDENT) {
        break;
      }
    }

    ast->addChild(std::move(decl));
  }

  return ast;
}

AstPtr SdtBuilder::buildVarDeclaration(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto& children = node.children();

  for (std::size_t i = 0; i < children.size(); ++i) {
    if (children[i]->type() != NodeType::IdentifierList) continue;

    const auto names = identifierList(*children[i]);
    const parser::ParseNode* type_node = nullptr;
    for (std::size_t j = i + 1; j < children.size(); ++j) {
      if (children[j]->type() == NodeType::Type) {
        type_node = children[j].get();
        i = j;
        break;
      }
      if (children[j]->type() == NodeType::IdentifierList) {
        break;
      }
    }

    for (const auto& name : names) {
      auto decl = std::make_unique<AstNode>(AstKind::VarDecl);
      decl->setName(name);
      if (type_node) {
        decl->addChild(buildType(*type_node));
      } else {
        decl->addChild(makeError());
      }
      ast->addChild(std::move(decl));
    }
  }

  return ast;
}

AstPtr SdtBuilder::buildType(const parser::ParseNode& node) const {
  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT) {
      return named_type_from_token(*token);
    }

    switch (child->type()) {
      case NodeType::ArrayType:
        return buildArrayType(*child);
      case NodeType::Range:
        return buildRange(*child);
      case NodeType::Enumerated:
        return buildEnumerated(*child);
      case NodeType::RecordType:
        return buildRecordType(*child);
      case NodeType::Error:
        return makeError();
      default:
        break;
    }
  }
  return makeError();
}

AstPtr SdtBuilder::buildArrayType(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::ArrayType);
  const parser::ParseNode* element_type = nullptr;
  bool in_index = false;

  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child)) {
      if (token->type == TokenType::LBRACK) {
        in_index = true;
      } else if (token->type == TokenType::RBRACK) {
        in_index = false;
      } else if (in_index && token->type == TokenType::IDENT) {
        ast->addChild(named_type_from_token(*token));
      }
      continue;
    }

    if (child->type() == NodeType::Range && in_index) {
      ast->addChild(buildRange(*child));
    } else if (child->type() == NodeType::Type) {
      element_type = child.get();
    }
  }

  if (element_type) {
    ast->addChild(buildType(*element_type));
  } else {
    ast->addChild(makeError());
  }

  return ast;
}

AstPtr SdtBuilder::buildRange(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::RangeType);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::Constant) {
      ast->addChild(buildConstant(*child));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildEnumerated(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::EnumType);
  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT) {
      auto literal = std::make_unique<AstNode>(AstKind::Literal, *token);
      literal->setName(token->lexeme);
      literal->annotation().type = "enumerated";
      ast->addChild(std::move(literal));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildRecordType(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::RecordType);
  if (const auto* fields = firstChildOf(node, NodeType::FieldList)) {
    ast->addChild(buildFieldList(*fields));
  }
  return ast;
}

AstPtr SdtBuilder::buildFieldList(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::FieldPart) {
      ast->addChild(buildFieldPart(*child));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildFieldPart(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto* ids = firstChildOf(node, NodeType::IdentifierList);
  const auto* type = firstChildOf(node, NodeType::Type);

  if (!ids) return makeError();

  for (const auto& name : identifierList(*ids)) {
    auto field = std::make_unique<AstNode>(AstKind::FieldDecl);
    field->setName(name);
    if (type) {
      field->addChild(buildType(*type));
    } else {
      field->addChild(makeError());
    }
    ast->addChild(std::move(field));
  }

  return ast;
}

AstPtr SdtBuilder::buildConstant(const parser::ParseNode& node) const {
  std::string sign;
  for (const auto& child : node.children()) {
    const auto* token = tokenOf(*child);
    if (!token) continue;

    if (token->type == TokenType::PLUS || token->type == TokenType::MINUS) {
      sign = tokenOperator(*token);
      continue;
    }

    if (is_ignored_declaration_token(token->type)) continue;

    if (token->type == TokenType::IDENT) {
      auto ident = std::make_unique<AstNode>(AstKind::VarRef, *token);
      ident->setName(token->lexeme);
      if (sign == "-") {
        auto unary = std::make_unique<AstNode>(AstKind::UnaryOp);
        unary->setOp("-");
        unary->addChild(std::move(ident));
        return unary;
      }
      return ident;
    }

    auto literal = std::make_unique<AstNode>(AstKind::Literal, *token);
    literal->setValue(sign + token->lexeme);
    literal->annotation().type = literalType(*token);
    return literal;
  }
  return makeError();
}

}  // namespace semantic
