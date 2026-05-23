#pragma once

#include <iosfwd>
#include <string>

#include "ast/ast_visitor.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "ast/type_nodes.hpp"

namespace semantic {

/**
 * @brief Prints a decorated AST as an indented tree with UTF-8 or ASCII
 * connectors.
 *
 */
class ASTPrinter : public ast::ASTVisitor {
 public:
  explicit ASTPrinter(std::ostream &os, bool ascii = false);

  void print(ast::AstNode &root);

  void visit(ast::ProgramNode &node) override;
  void visit(ast::BlockNode &node) override;

  void visit(ast::SimpleTypeSpecNode &node) override;
  void visit(ast::SubrangeTypeSpecNode &node) override;
  void visit(ast::ArrayTypeSpecNode &node) override;
  void visit(ast::RecordTypeSpecNode &node) override;
  void visit(ast::EnumTypeSpecNode &node) override;

  void visit(ast::ConstDeclNode &node) override;
  void visit(ast::VarDeclNode &node) override;
  void visit(ast::TypeDeclNode &node) override;
  void visit(ast::ProcDeclNode &node) override;
  void visit(ast::FuncDeclNode &node) override;

  void visit(ast::BinOpNode &node) override;
  void visit(ast::UnaryOpNode &node) override;
  void visit(ast::NumberNode &node) override;
  void visit(ast::StringNode &node) override;
  void visit(ast::IdentNode &node) override;
  void visit(ast::FuncCallNode &node) override;
  void visit(ast::ArrayAccessNode &node) override;
  void visit(ast::RecordAccessNode &node) override;

  void visit(ast::AssignNode &node) override;
  void visit(ast::IfNode &node) override;
  void visit(ast::WhileNode &node) override;
  void visit(ast::RepeatNode &node) override;
  void visit(ast::ForNode &node) override;
  void visit(ast::ProcCallNode &node) override;
  void visit(ast::CompoundStmtNode &node) override;

 private:
  std::ostream &os_;
  bool ascii_;
  std::string prefix_;
  bool is_last_ = true;
  bool is_root_ = true;

  const char *branch() const;
  const char *lastBranch() const;
  const char *pipe() const;
  const char *blank() const;
  const char *arrow() const;

  /**
   * @brief Print one node line. Manages is_root_ state.
   *
   * @param label
   * @param ann
   */
  void printHead(const std::string &label, const std::string &ann);

  /**
   * @brief Common decoration annotation: "type:N, tab:N" (omits zeros).
   *
   * @param n
   * @return std::string
   */
  static std::string nodeAnnotation(const ast::AstNode &n);

  /**
   * @brief Call before visiting a child group. Returns saved prefix for
   * restore.
   *
   * @param wasRoot
   * @param wasLast
   * @return std::string
   */
  std::string enterChildren(bool wasRoot, bool wasLast);

  /**
   * @brief Restores the prefix saved by enterChildren.
   *
   * @param saved
   */
  void leaveChildren(const std::string &saved);

  /**
   * @brief Helper: visit a single child with is_last_ set appropriately.
   *
   * @param child
   * @param is_last
   */
  void visitChild(ast::AstNode &child, bool is_last);

  // Helper: visit a list of children.
  /**
   * @brief Helper: visit a list of children.
   *
   * @tparam PtrVec a vector of unique_ptrs to AST nodes
   * @param vec the vector of children to visit
   */
  template <typename PtrVec>
  void visitChildren(const PtrVec &vec);
};

}  // namespace semantic
