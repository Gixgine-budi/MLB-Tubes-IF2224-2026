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

  // Main entry point for Syntax-Directed Translation
  Ptr<ast::AstNode> build(const parser::ParseNode &parse_node);

 private:
  // Helper translation methods for converting ParseNode to discrete AstNode
  // types
  Ptr<ast::ProgramNode> buildProgram(const parser::ParseNode &node);
  Ptr<ast::BlockNode> buildBlock(const parser::ParseNode &node);

  // Declarations
  std::vector<Ptr<ast::AstNode>> buildDeclarations(
      const parser::ParseNode &node);
  Ptr<ast::VarDeclNode> buildVarDecl(const parser::ParseNode &node);
  Ptr<ast::TypeDeclNode> buildTypeDecl(const parser::ParseNode &node);
  Ptr<ast::ProcDeclNode> buildProcDecl(const parser::ParseNode &node);
  Ptr<ast::FuncDeclNode> buildFuncDecl(const parser::ParseNode &node);
  std::vector<Ptr<ast::ParameterNode>> buildFormalParameters(
      const parser::ParseNode &node);

  // Statements
  Ptr<ast::StmtNode> buildStatement(const parser::ParseNode &node);
  Ptr<ast::CompoundStmtNode> buildCompoundStmt(const parser::ParseNode &node);
  Ptr<ast::AssignNode> buildAssign(const parser::ParseNode &node);
  Ptr<ast::IfNode> buildIf(const parser::ParseNode &node);
  Ptr<ast::WhileNode> buildWhile(const parser::ParseNode &node);
  Ptr<ast::RepeatNode> buildRepeat(const parser::ParseNode &node);
  Ptr<ast::ForNode> buildFor(const parser::ParseNode &node);
  Ptr<ast::ProcCallNode> buildProcCall(const parser::ParseNode &node);

  // Expressions
  Ptr<ast::ExprNode> buildExpression(const parser::ParseNode &node);
  Ptr<ast::ExprNode> buildSimpleExpression(const parser::ParseNode &node);
  Ptr<ast::ExprNode> buildTerm(const parser::ParseNode &node);
  Ptr<ast::ExprNode> buildFactor(const parser::ParseNode &node);

  // Variables & accessing (arrays, records)
  Ptr<ast::ExprNode> buildVariableAccess(const parser::ParseNode &node);
};

}  // namespace semantic
