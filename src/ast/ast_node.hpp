#pragma once

#include <optional>

#include "ast_visitor.hpp"
#include "lexer/token.hpp"

namespace ast {

class AstNode {
 public:
  virtual ~AstNode() = default;

  /**
   * @brief Visitor pattern accept method for AST traversal. Each concrete node
   * will implement this to call the appropriate visit method on the visitor.
   *
   * @param visitor The visitor instance that will visit this node.
   */
  virtual void accept(ASTVisitor& visitor) = 0;

  // Optional: Location tracking for error reporting
  std::optional<lexer::Token> token;

  /// Optional: Type decoration (populated during Semantic Analysis)
  int expression_type = 0;
};

}  // namespace ast
