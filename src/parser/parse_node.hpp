#pragma once

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "lexer/token.hpp"

namespace parser {

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

class ParseNode {
 public:
  ParseNode(NodeType type) : type_(type) {}
  ParseNode(NodeType type, lexer::Token token) : type_(type), token_(token) {}

  NodeType type() const { return type_; }
  const std::optional<lexer::Token>& token() const { return token_; }
  const std::vector<std::unique_ptr<ParseNode>>& children() const {
    return children_;
  }

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