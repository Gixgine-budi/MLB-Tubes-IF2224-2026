#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

enum class AstKind {
  Program,
  ProgramHeader,
  Declarations,
  ConstDecl,
  TypeDecl,
  VarDecl,
  NamedType,
  ArrayType,
  RangeType,
  EnumType,
  RecordType,
  FieldDecl,
  Block,
  CompoundStatement,
  StatementList,
  Statement,
  Assign,
  If,
  Case,
  CaseBranch,
  While,
  Repeat,
  For,
  Call,
  ParameterList,
  VarRef,
  ArrayAccess,
  FieldAccess,
  Literal,
  UnaryOp,
  BinOp,
  Empty,
  Error,
};

struct AstAnnotation {
  std::string type;
  int tab_index = -1;
  int block_index = -1;
  int array_index = -1;
  int lexical_level = -1;
};

class AstNode;
using AstPtr = std::unique_ptr<AstNode>;

class AstNode {
 public:
  explicit AstNode(AstKind kind);
  AstNode(AstKind kind, std::string value);
  AstNode(AstKind kind, const lexer::Token& token);

  AstKind kind() const { return kind_; }
  const std::string& name() const { return name_; }
  const std::string& value() const { return value_; }
  const std::string& op() const { return op_; }
  const std::optional<lexer::Token>& token() const { return token_; }
  const std::vector<AstPtr>& children() const { return children_; }
  const AstAnnotation& annotation() const { return annotation_; }

  void setName(std::string name) { name_ = std::move(name); }
  void setValue(std::string value) { value_ = std::move(value); }
  void setOp(std::string op) { op_ = std::move(op); }
  AstAnnotation& annotation() { return annotation_; }

  AstNode& addChild(AstPtr child);
  void print(std::ostream& os, int indent = 0) const;

 private:
  AstKind kind_;
  std::string name_;
  std::string value_;
  std::string op_;
  std::optional<lexer::Token> token_;
  std::vector<AstPtr> children_;
  AstAnnotation annotation_;
};

std::ostream& operator<<(std::ostream& os, AstKind kind);
std::ostream& operator<<(std::ostream& os, const AstNode& node);

class SdtBuilder {
 public:
  AstPtr build(const parser::ParseNode& node) const;

  AstPtr buildProgram(const parser::ParseNode& node) const;
  AstPtr buildProgramHeader(const parser::ParseNode& node) const;
  AstPtr buildDeclarationPart(const parser::ParseNode& node) const;
  AstPtr buildConstDeclaration(const parser::ParseNode& node) const;
  AstPtr buildTypeDeclaration(const parser::ParseNode& node) const;
  AstPtr buildVarDeclaration(const parser::ParseNode& node) const;
  AstPtr buildType(const parser::ParseNode& node) const;
  AstPtr buildArrayType(const parser::ParseNode& node) const;
  AstPtr buildRange(const parser::ParseNode& node) const;
  AstPtr buildEnumerated(const parser::ParseNode& node) const;
  AstPtr buildRecordType(const parser::ParseNode& node) const;
  AstPtr buildFieldList(const parser::ParseNode& node) const;
  AstPtr buildFieldPart(const parser::ParseNode& node) const;
  AstPtr buildConstant(const parser::ParseNode& node) const;

  AstPtr buildExpression(const parser::ParseNode& node) const;
  AstPtr buildSimpleExpression(const parser::ParseNode& node) const;
  AstPtr buildTerm(const parser::ParseNode& node) const;
  AstPtr buildFactor(const parser::ParseNode& node) const;
  AstPtr buildVariable(const parser::ParseNode& node) const;
  AstPtr buildComponentVariable(AstPtr base, const parser::ParseNode& node) const;
  AstPtr buildIndexElement(const parser::ParseNode& node) const;
  AstPtr buildFunctionCall(const parser::ParseNode& node) const;
  AstPtr buildParameterList(const parser::ParseNode& node) const;

 private:
  static bool isToken(const parser::ParseNode& node);
  static bool isToken(const parser::ParseNode& node, lexer::TokenType type);
  static const lexer::Token* tokenOf(const parser::ParseNode& node);
  static const parser::ParseNode* childAt(const parser::ParseNode& node,
                                          std::size_t index);
  static const parser::ParseNode* firstChildOf(
      const parser::ParseNode& node, parser::NodeType type);
  static std::vector<std::string> identifierList(
      const parser::ParseNode& node);
  static std::string tokenOperator(const lexer::Token& token);
  static std::string literalType(const lexer::Token& token);
  static AstPtr makeError();
  static AstPtr makeEmpty();
};

}  // namespace semantic
