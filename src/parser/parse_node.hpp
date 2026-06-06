#pragma once

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "lexer/token.hpp"

namespace parser {

/**
 * @brief Enum representing type of parse node in the Parse Tree
 *
 */
enum class NodeType {
  Program,
  ProgramHeader,
  DeclarationPart,
  ConstDeclaration,
  Constant,
  TypeDeclaration,
  VarDeclaration,
  IdentifierList,
  Type,
  ArrayType,
  Range,
  Enumerated,
  RecordType,
  FieldList,
  FieldPart,
  SubprogramDeclaration,
  ProcedureDeclaration,
  FunctionDeclaration,
  Block,
  FormalParameterList,
  ParameterGroup,
  CompoundStatement,
  StatementList,
  Statement,
  Variable,
  ComponentVariable,
  IndexList,
  AssignmentStatement,
  IfStatement,
  CaseStatement,
  CaseBlock,
  WhileStatement,
  RepeatStatement,
  ForStatement,
  FunctionCall,
  ParameterList,
  Expression,
  SimpleExpression,
  Term,
  Factor,
  RelationalOperator,
  AdditiveOperator,
  MultiplicativeOperator,
  TokenNode,
  Error,
};

/**
 * @brief Generic parse node class representing a node in the Parse Tree
 *
 */
class ParseNode {
 public:
  ParseNode(NodeType type) : type_(type) {}
  ParseNode(NodeType type, lexer::Token token) : type_(type), token_(token) {}

  /**
   * @brief Return the type of the parse node
   *
   * @return NodeType
   */
  NodeType type() const { return type_; }

  /**
   * @brief Returns the token associated with this parse node, if any. Only
   * valid for terminal nodes (TokenNode).
   *
   * @return const std::optional<lexer::Token>&
   */
  const std::optional<lexer::Token>& token() const { return token_; }

  /**
   * @brief Return the list of child nodes of this parse node
   *
   * @return const std::vector<std::unique_ptr<ParseNode>>&
   */
  const std::vector<std::unique_ptr<ParseNode>>& children() const {
    return children_;
  }

  /**
   * @brief Insert a child node to the current node.
   *
   * @param child unique pointer to the child node to be added. The current node
   * takes ownership of the child node.
   */
  void addChild(std::unique_ptr<ParseNode> child) {
    children_.push_back(std::move(child));
  }

  /**
   * @brief Output the node type and token (if any) in a human-readable format
   *
   * @param os the output stream
   * @param node the parse node to be printed
   * @return std::ostream& the (modified) output stream
   */
  friend std::ostream& operator<<(std::ostream& os, const ParseNode& node);

  /**
   * @brief Output the parse tree recursively to the output stream.
   *
   * @param out The output stream
   * @param prefix The prefix string used for formatting the tree structure
   * @param isLast Indicates if the current node is the last child of its parent
   * @param ascii Print in ascii compatible format, otherwise print with
   * non-ascii characters
   */
  void print(std::ostream& out, const std::string& prefix, bool isLast,
             bool ascii = false) const;

 private:
  NodeType type_;
  std::optional<lexer::Token> token_;
  std::vector<std::unique_ptr<ParseNode>> children_;
};

}  // namespace parser