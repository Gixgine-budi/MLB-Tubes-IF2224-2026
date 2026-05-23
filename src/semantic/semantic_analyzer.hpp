#pragma once

#include "ast/ast_visitor.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "semantic/symbol_table.hpp"

namespace semantic {

class SemanticAnalyzer : public ast::ASTVisitor {
 public:
  SemanticAnalyzer() = default;
  ~SemanticAnalyzer() override = default;

  // Entry point for Semantic Analysis pass
  void analyze(ast::AstNode &root);

  // Provide access to the populated environment structure after analysis
  const SymbolTable &getSymbolTable() const { return sym_table; }

  // --- Visitor Overrides ---

  // Program and Block structure

  void visit(ast::ProgramNode &node) override;
  void visit(ast::BlockNode &node) override;

  // Type Specifications

  void visit(ast::SimpleTypeSpecNode &node) override;
  void visit(ast::SubrangeTypeSpecNode &node) override;
  void visit(ast::ArrayTypeSpecNode &node) override;
  void visit(ast::RecordTypeSpecNode &node) override;
  void visit(ast::EnumTypeSpecNode &node) override;

  // Declarations

  void visit(ast::ConstDeclNode &node) override;
  void visit(ast::VarDeclNode &node) override;
  void visit(ast::TypeDeclNode &node) override;
  void visit(ast::ProcDeclNode &node) override;
  void visit(ast::FuncDeclNode &node) override;

  // Expressions

  void visit(ast::BinOpNode &node) override;
  void visit(ast::UnaryOpNode &node) override;
  void visit(ast::NumberNode &node) override;
  void visit(ast::StringNode &node) override;
  void visit(ast::IdentNode &node) override;
  void visit(ast::FuncCallNode &node) override;
  void visit(ast::ArrayAccessNode &node) override;
  void visit(ast::RecordAccessNode &node) override;

  // Statements

  void visit(ast::AssignNode &node) override;
  void visit(ast::IfNode &node) override;
  void visit(ast::WhileNode &node) override;
  void visit(ast::RepeatNode &node) override;
  void visit(ast::ForNode &node) override;
  void visit(ast::ProcCallNode &node) override;
  void visit(ast::CompoundStmtNode &node) override;

 private:
  SymbolTable sym_table;
  bool has_errors_ = false;

  void reportError(const std::string &message);
  int resolveTypeSpec(ast::TypeSpecNode &spec);
  int resolveSimpleTypeName(const std::string &name);
  bool typesCompatible(int left, int right) const;
  bool assignmentCompatible(int target, int value) const;
  void visitExpr(ast::ExprNode &expr);
  void visitStmt(ast::StmtNode &stmt);

  void checkTypeCompatibility(int expected_type, int actual_type,
                              const std::string &context);
  void enterScope();
  void leaveScope();
  int get_base_type(const std::string &type_name);
  bool isAssignmentCompatible(int target_type, int expr_type);
};

}  // namespace semantic
