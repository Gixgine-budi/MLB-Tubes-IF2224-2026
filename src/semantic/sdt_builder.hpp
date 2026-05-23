#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "ast/ast_node.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "diagnoser/diagnoser.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

template <typename T>
using Ptr = std::unique_ptr<T>;

class SDTBuilder {
 public:
  /**
   * @brief Construct a builder bound to one parse-tree root.
   *
   * The parse root and diagnoser are borrowed references and must outlive this
   * SDTBuilder instance.
   */
  SDTBuilder(const parser::ParseNode &parse_root, diag::Diagnoser &diagnoser);
  ~SDTBuilder() = default;

  /**
   * @brief Translate the bound parse-tree root into an AST root.
   *
   * The previous AST (if any) will be replaced. Semantic-phase diagnostics are
   * accumulated into the bound diagnoser.
   */
  void build();

  /**
   * @brief Print current AST to stdout for debugging.
   *
   * @param ascii true for ASCII tree style, false for Unicode style.
   */
  void print(bool ascii = false) const;

  /**
   * @brief Print current AST to a stream for debugging.
   *
   * @param os output stream target.
   * @param ascii true for ASCII tree style, false for Unicode style.
   */
  void print(std::ostream &os, bool ascii = false) const;

  /**
   * @brief Whether build() has produced a non-null AST root.
   */
  bool hasAst() const { return ast_root_ != nullptr; }

  /**
   * @brief Whether this builder has emitted semantic diagnostics.
   */
  bool hasErrors() const { return has_errors_; }

  /**
   * @brief Access built AST root.
   *
   * @return const reference to AST root.
   * @throws std::logic_error if build() has not produced an AST yet.
   */
  const ast::AstNode &getAst() const;

 private:
  const parser::ParseNode &parse_root_;
  diag::Diagnoser &diagnoser_;
  Ptr<ast::AstNode> ast_root_;
  bool built_ = false;
  bool has_errors_ = false;

  void reportBuildError(const parser::ParseNode &node,
                        const std::string &message,
                        const std::string &hint = "");

  const lexer::Token *firstToken(const parser::ParseNode &node) const;

  /**
   * @brief Build ast::ProgramNode from a Program parse node.
   *
   * @param node Parse node of type NodeType::Program.
   * @return Program AST node with identifier and owned Block.
   */
  Ptr<ast::ProgramNode> buildProgram(const parser::ParseNode &node);

  /**
   * @brief Build ast::BlockNode from a Block parse node.
   *
   * @param node Parse node of type NodeType::Block.
   * @return Block AST node containing declaration list and compound statement.
   */
  Ptr<ast::BlockNode> buildBlock(const parser::ParseNode &node);

  // --------------------------- sdt_type_spec.cpp ---------------------------

  /**
   * @brief Build type specification AST node.
   *
   * Dispatches by parse node kind and lowers aliases, subranges, arrays,
   * enumerations, and record types.
   *
   * Implemented in sdt_type_spec.cpp.
   *
   * @param node Parse node for a type specification form.
   * @return Type specification AST node, or nullptr if unsupported/invalid.
   */
  Ptr<ast::TypeSpecNode> buildTypeSpec(const parser::ParseNode &node);

  /**
   * @brief Build simple type specification from an identifier token node.
   *
   * @param node Parse node for a range type specification.
   * @return Type specification AST node, or nullptr if unsupported/invalid.
   */
  Ptr<ast::TypeSpecNode> buildRangeTypeSpec(const parser::ParseNode &node);

  /**
   * @brief Build array type specification from an array type parse node.
   *
   * @param node Parse node for an array type specification.
   * @return Type specification AST node, or nullptr if unsupported/invalid.
   */
  Ptr<ast::TypeSpecNode> buildArrayTypeSpec(const parser::ParseNode &node);

  /**
   * @brief Build enumerated type specification from an enumerated type parse
   * node.
   *
   * @param node Parse node for an enumerated type.
   * @return Type specification AST node, or nullptr if unsupported/invalid.
   */
  Ptr<ast::TypeSpecNode> buildEnumeratedTypeSpec(const parser::ParseNode &node);

  /**
   * @brief Build record type specification from a record type parse node.
   *
   * @param node Parse node for a record type specification.
   * @return Type specification AST node, or nullptr if unsupported/invalid.
   */
  Ptr<ast::TypeSpecNode> buildRecordTypeSpec(const parser::ParseNode &node);

  // -------------------------- sdt_declaration.cpp --------------------------

  /**
   * @brief Lower DeclarationPart into a flat AST declaration list.
   *
   * @param node Parse node of type NodeType::DeclarationPart.
   * @return Ordered declaration AST nodes for BlockNode::declarations.
   */
  std::vector<Ptr<ast::AstNode>> buildDeclarations(
      const parser::ParseNode &node);

  /**
   * @brief Build constant expression node for declaration contexts.
   *
   * Supports signed numeric constants, literal constants, and identifier
   * constants used by const declarations and subrange bounds.
   *
   * @param node Parse node of type NodeType::Constant.
   * @return Expression node for constant context, or nullptr if invalid.
   */
  Ptr<ast::ExprNode> buildConstantExpr(const parser::ParseNode &node);

  /**
   * @brief Build one variable declaration AST node.
   *
   * @param node Parse node representing one var declaration unit.
   * @return Variable declaration AST node.
   */
  Ptr<ast::VarDeclNode> buildVarDecl(const parser::ParseNode &node);

  /**
   * @brief Build one type declaration AST node.
   *
   * @param node Parse node representing one type declaration unit.
   * @return Type declaration AST node.
   */
  Ptr<ast::TypeDeclNode> buildTypeDecl(const parser::ParseNode &node);

  /**
   * @brief Build procedure declaration AST node.
   *
   * @param node Parse node of type NodeType::ProcedureDeclaration.
   * @return Procedure declaration AST node with parameters and block body.
   */
  Ptr<ast::ProcDeclNode> buildProcDecl(const parser::ParseNode &node);

  /**
   * @brief Build function declaration AST node.
   *
   * @param node Parse node of type NodeType::FunctionDeclaration.
   * @return Function declaration AST node.
   */
  Ptr<ast::FuncDeclNode> buildFuncDecl(const parser::ParseNode &node);

  /**
   * @brief Build parameter groups from a formal parameter list.
   *
   * @param node Parse node of type NodeType::FormalParameterList.
   * @return Parameter nodes in declaration order.
   */
  std::vector<Ptr<ast::ParameterNode>> buildFormalParameters(
      const parser::ParseNode &node);

  // --------------------------- sdt_statement.cpp ---------------------------

  /**
   * @brief Dispatch one Statement parse node into concrete AST statement node.
   *
   * @param node Parse node of type NodeType::Statement.
   * @return Concrete statement AST node, or nullptr for empty/error statement.
   */
  Ptr<ast::StmtNode> buildStatement(const parser::ParseNode &node);

  /**
   * @brief Build ast::CompoundStmtNode from BEGIN..END parse structure.
   *
   * @param node Parse node of type NodeType::CompoundStatement.
   * @return Compound statement AST node.
   */
  Ptr<ast::CompoundStmtNode> buildCompoundStmt(const parser::ParseNode &node);

  /**
   * @brief Build assignment statement AST node.
   *
   * @param node Parse node of type NodeType::AssignmentStatement.
   * @return Assignment AST node.
   */
  Ptr<ast::AssignNode> buildAssign(const parser::ParseNode &node);

  /**
   * @brief Build if/then[/else] statement AST node.
   *
   * @param node Parse node of type NodeType::IfStatement.
   * @return If statement AST node.
   */
  Ptr<ast::IfNode> buildIf(const parser::ParseNode &node);

  /**
   * @brief Build case/of statement by lowering it into nested If nodes.
   *
   * @param node Parse node of type NodeType::CaseStatement.
   * @return Lowered statement AST node.
   */
  Ptr<ast::StmtNode> buildCase(const parser::ParseNode &node);

  /**
   * @brief Build case-label constant expression node.
   *
   * Supports signed numeric labels, string/char labels, and identifier labels
   * used by case-of branches.
   *
   * @param node Parse node of type NodeType::Constant.
   * @return Expression node for a single case label, or nullptr if invalid.
   */
  Ptr<ast::ExprNode> buildCaseConstantExpr(const parser::ParseNode &node);

  /**
   * @brief Build while/do statement AST node.
   *
   * @param node Parse node of type NodeType::WhileStatement.
   * @return While statement AST node.
   */
  Ptr<ast::WhileNode> buildWhile(const parser::ParseNode &node);

  /**
   * @brief Build repeat/until statement AST node.
   *
   * @param node Parse node of type NodeType::RepeatStatement.
   * @return Repeat statement AST node.
   */
  Ptr<ast::RepeatNode> buildRepeat(const parser::ParseNode &node);

  /**
   * @brief Build for loop statement AST node.
   *
   * @param node Parse node of type NodeType::ForStatement.
   * @return For statement AST node.
   */
  Ptr<ast::ForNode> buildFor(const parser::ParseNode &node);

  /**
   * @brief Build procedure-call statement AST node.
   *
   * @param node Parse node for call statement form.
   * @return Procedure call AST node.
   */
  Ptr<ast::ProcCallNode> buildProcCall(const parser::ParseNode &node);

  // --------------------------- sdt_expression.cpp ---------------------------

  /**
   * @brief Build expression with optional relational operator.
   *
   * @param node Parse node of type NodeType::Expression.
   * @return Expression AST root.
   */
  Ptr<ast::ExprNode> buildExpression(const parser::ParseNode &node);

  /**
   * @brief Build additive-precedence expression tree.
   *
   * @param node Parse node of type NodeType::SimpleExpression.
   * @return Expression subtree for additive precedence level.
   */
  Ptr<ast::ExprNode> buildSimpleExpression(const parser::ParseNode &node);

  /**
   * @brief Build multiplicative-precedence expression tree.
   *
   * @param node Parse node of type NodeType::Term.
   * @return Expression subtree for multiplicative precedence level.
   */
  Ptr<ast::ExprNode> buildTerm(const parser::ParseNode &node);

  /**
   * @brief Build atomic expression factor.
   *
   * @param node Parse node of type NodeType::Factor.
   * @return Atomic expression AST node.
   */
  Ptr<ast::ExprNode> buildFactor(const parser::ParseNode &node);

  /**
   * @brief Build variable access chain (identifier, indexing, field access).
   *
   * @param node Parse node of type NodeType::Variable.
   * @return Expression node representing full l-value/r-value access path.
   */
  Ptr<ast::ExprNode> buildVariableAccess(const parser::ParseNode &node);
};

}  // namespace semantic
