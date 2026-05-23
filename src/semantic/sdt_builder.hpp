#pragma once

#include <memory>
#include <vector>

#include "ast/ast_node.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

template <typename T>
using Ptr = std::unique_ptr<T>;

class SDTBuilder {
 public:
  SDTBuilder() = default;
  ~SDTBuilder() = default;

  /**
   * @brief Translate a parse-tree root into an AST root.
   *
   * @param parse_node Parse tree root produced by the parser.
   * @return AST root node (normally ast::ProgramNode), or nullptr on mismatch.
   */
  Ptr<ast::AstNode> build(const parser::ParseNode &parse_node);

 private:
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

  // -------------------------- sdt_declaration.cpp --------------------------

  /**
   * @brief Lower DeclarationPart into a flat AST declaration list.
   *
   * @param node Parse node of type NodeType::DeclarationPart.
   * @return Ordered declaration AST nodes for BlockNode::declarations.
   */
  std::vector<Ptr<ast::AstNode>> buildDeclarations(
      const parser::ParseNode &node);
  Ptr<ast::TypeSpecNode> buildTypeSpec(const parser::ParseNode &node);

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
