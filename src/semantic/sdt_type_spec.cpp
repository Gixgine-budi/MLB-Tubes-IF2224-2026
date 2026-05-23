#include <utility>
#include <vector>

#include "ast/ast_node.hpp"
#include "ast/type_nodes.hpp"
#include "lexer/token.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {

Ptr<ast::TypeSpecNode> SDTBuilder::buildTypeSpec(
    const parser::ParseNode &node) {
  using parser::NodeType;

  switch (node.type()) {
    case NodeType::Type:
      if (node.children().empty()) {
        reportBuildError(node, "type wrapper node has no child");
        return nullptr;
      }
      return buildTypeSpec(*node.children()[0]);

    case NodeType::TokenNode:
      if (node.token().has_value() &&
          node.token()->type == lexer::TokenType::IDENT) {
        return std::make_unique<ast::SimpleTypeSpecNode>(node.token().value());
      }
      reportBuildError(node, "type token node is not an identifier");
      return nullptr;

    case NodeType::Range:
      return buildRangeTypeSpec(node);

    case NodeType::ArrayType:
      return buildArrayTypeSpec(node);

    case NodeType::Enumerated:
      return buildEnumeratedTypeSpec(node);

    case NodeType::RecordType:
      return buildRecordTypeSpec(node);

    default:
      reportBuildError(node, "unsupported type specification node");
      return nullptr;
  }
}

Ptr<ast::TypeSpecNode> SDTBuilder::buildRangeTypeSpec(
    const parser::ParseNode &node) {
  if (node.children().size() < 4) {
    reportBuildError(node, "range type has incomplete bounds");
    return nullptr;
  }

  std::unique_ptr<ast::AstNode> low = buildConstantExpr(*node.children()[0]);
  std::unique_ptr<ast::AstNode> high = buildConstantExpr(*node.children()[3]);
  if (low == nullptr || high == nullptr) {
    reportBuildError(node, "failed to build range bound constants");
    return nullptr;
  }
  return std::make_unique<ast::SubrangeTypeSpecNode>(std::move(low),
                                                     std::move(high));
}

Ptr<ast::TypeSpecNode> SDTBuilder::buildArrayTypeSpec(
    const parser::ParseNode &node) {
  using parser::NodeType;

  if (node.children().size() < 6) {
    reportBuildError(node, "array type node has incomplete structure");
    return nullptr;
  }

  Ptr<ast::TypeSpecNode> index_type;
  const auto &index_node = *node.children()[2];
  if (index_node.type() == NodeType::TokenNode && index_node.token() &&
      index_node.token()->type == lexer::TokenType::IDENT) {
    index_type =
        std::make_unique<ast::SimpleTypeSpecNode>(index_node.token().value());
  } else {
    index_type = buildTypeSpec(index_node);
  }

  auto element_type = buildTypeSpec(*node.children()[5]);
  if (index_type == nullptr || element_type == nullptr) {
    reportBuildError(node, "failed to build array index or element type");
    return nullptr;
  }
  return std::make_unique<ast::ArrayTypeSpecNode>(std::move(index_type),
                                                  std::move(element_type));
}

Ptr<ast::TypeSpecNode> SDTBuilder::buildEnumeratedTypeSpec(
    const parser::ParseNode &node) {
  using parser::NodeType;

  std::vector<lexer::Token> literals;
  literals.reserve(node.children().size());
  for (const auto &child : node.children()) {
    if (child->type() == NodeType::TokenNode && child->token() &&
        child->token()->type == lexer::TokenType::IDENT) {
      literals.push_back(child->token().value());
    }
  }
  if (literals.empty()) {
    reportBuildError(node, "enumerated type has no identifier literals");
  }
  return std::make_unique<ast::EnumTypeSpecNode>(std::move(literals));
}

Ptr<ast::TypeSpecNode> SDTBuilder::buildRecordTypeSpec(
    const parser::ParseNode &node) {
  using parser::NodeType;

  std::vector<
      std::pair<std::vector<lexer::Token>, std::unique_ptr<ast::TypeSpecNode>>>
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

  if (fields.empty()) {
    reportBuildError(node, "record type has no field declarations");
  }

  return std::make_unique<ast::RecordTypeSpecNode>(std::move(fields));
}

}  // namespace semantic
