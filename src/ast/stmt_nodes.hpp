#pragma once

#include <memory>
#include <vector>

#include "ast/ast_node.hpp"
#include "ast/expr_nodes.hpp"

namespace ast {

class StmtNode : public AstNode {
 public:
  virtual ~StmtNode() = default;
};

class AssignNode : public StmtNode {
 public:
  std::unique_ptr<ExprNode> target;
  std::unique_ptr<ExprNode> expr;

  AssignNode(std::unique_ptr<ExprNode> t, std::unique_ptr<ExprNode> e)
      : target(std::move(t)), expr(std::move(e)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class IfNode : public StmtNode {
 public:
  std::unique_ptr<ExprNode> condition;
  std::unique_ptr<StmtNode> then_branch;
  std::unique_ptr<StmtNode> else_branch;  // Can be nullptr

  IfNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<StmtNode> then_b,
         std::unique_ptr<StmtNode> else_b = nullptr)
      : condition(std::move(cond)),
        then_branch(std::move(then_b)),
        else_branch(std::move(else_b)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class WhileNode : public StmtNode {
 public:
  std::unique_ptr<ExprNode> condition;
  std::unique_ptr<StmtNode> body;

  WhileNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<StmtNode> b)
      : condition(std::move(cond)), body(std::move(b)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class RepeatNode : public StmtNode {
 public:
  std::vector<std::unique_ptr<StmtNode>> statements;
  std::unique_ptr<ExprNode> condition;

  RepeatNode(std::vector<std::unique_ptr<StmtNode>> stmts,
             std::unique_ptr<ExprNode> cond)
      : statements(std::move(stmts)), condition(std::move(cond)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ForNode : public StmtNode {
 public:
  lexer::Token iterator;
  std::unique_ptr<ExprNode> initial;
  std::unique_ptr<ExprNode> final;
  bool is_downto;
  std::unique_ptr<StmtNode> body;

  ForNode(lexer::Token iter, std::unique_ptr<ExprNode> start,
          std::unique_ptr<ExprNode> end, bool down, std::unique_ptr<StmtNode> b)
      : iterator(iter),
        initial(std::move(start)),
        final(std::move(end)),
        is_downto(down),
        body(std::move(b)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ProcCallNode : public StmtNode {
 public:
  lexer::Token id;
  std::vector<std::unique_ptr<ExprNode>> args;

  ProcCallNode(lexer::Token i, std::vector<std::unique_ptr<ExprNode>> a)
      : id(i), args(std::move(a)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class CompoundStmtNode : public StmtNode {
 public:
  std::vector<std::unique_ptr<StmtNode>> statements;

  CompoundStmtNode(std::vector<std::unique_ptr<StmtNode>> stmts)
      : statements(std::move(stmts)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

}  // namespace ast
