#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace semantic::sdt {

inline bool isToken(const parser::ParseNode& node) {
  return node.type() == parser::NodeType::TokenNode && node.token().has_value();
}

inline std::optional<lexer::Token> tokenOf(const parser::ParseNode& node) {
  if (isToken(node)) {
    return *node.token();
  }
  return std::nullopt;
}

inline const parser::ParseNode* findChild(const parser::ParseNode& parent,
                                          parser::NodeType type) {
  for (const auto& child : parent.children()) {
    if (child->type() == type) {
      return child.get();
    }
  }
  return nullptr;
}

inline std::vector<const parser::ParseNode*> childrenOfType(
    const parser::ParseNode& parent, parser::NodeType type) {
  std::vector<const parser::ParseNode*> out;
  for (const auto& child : parent.children()) {
    if (child->type() == type) {
      out.push_back(child.get());
    }
  }
  return out;
}

inline std::vector<lexer::Token> collectIdentifiers(
    const parser::ParseNode& id_list) {
  std::vector<lexer::Token> ids;
  for (const auto& child : id_list.children()) {
    if (auto tok = tokenOf(*child)) {
      if (tok->type == lexer::TokenType::IDENT) {
        ids.push_back(*tok);
      }
    }
  }
  return ids;
}

inline std::string lower(std::string s) {
  for (char& c : s) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return s;
}

}  // namespace semantic::sdt
