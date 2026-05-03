#pragma once

#include <memory>
#include <optional>
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

  void print(bool isLast, int depth = 0) const;

  friend std::ostream& operator<<(std::ostream& os, const ParseNode& node);

 private:
  NodeType type_;
  std::optional<lexer::Token> token_;
  std::vector<std::unique_ptr<ParseNode>> children_;
};

}  // namespace parser