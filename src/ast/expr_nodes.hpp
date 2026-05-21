#pragma once

#include <memory>
#include <vector>

#include "ast_node.hpp"

namespace ast {

class ExprNode : public AstNode {
 public:
  virtual ~ExprNode() = default;
};

class BinOpNode : public ExprNode {
 public:
  std::unique_ptr<ExprNode> left;
  lexer::Token op;
  std::unique_ptr<ExprNode> right;

  BinOpNode(std::unique_ptr<ExprNode> l, lexer::Token o,
            std::unique_ptr<ExprNode> r)
      : left(std::move(l)), op(o), right(std::move(r)) {}

  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class UnaryOpNode : public ExprNode {
 public:
  lexer::Token op;
  std::unique_ptr<ExprNode> expr;

  UnaryOpNode(lexer::Token o, std::unique_ptr<ExprNode> e)
      : op(o), expr(std::move(e)) {}

  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class NumberNode : public ExprNode {
 public:
  lexer::Token val;
  bool is_real;

  NumberNode(lexer::Token v, bool r = false) : val(v), is_real(r) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class StringNode : public ExprNode {
 public:
  lexer::Token val;

  StringNode(lexer::Token v) : val(v) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class IdentNode : public ExprNode {
 public:
  lexer::Token id;

  IdentNode(lexer::Token i) : id(i) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FuncCallNode : public ExprNode {
 public:
  lexer::Token id;
  std::vector<std::unique_ptr<ExprNode>> args;

  FuncCallNode(lexer::Token i, std::vector<std::unique_ptr<ExprNode>> a)
      : id(i), args(std::move(a)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayAccessNode : public ExprNode {
 public:
  std::unique_ptr<ExprNode> array_expr;
  std::vector<std::unique_ptr<ExprNode>> indices;

  ArrayAccessNode(std::unique_ptr<ExprNode> arr,
                  std::vector<std::unique_ptr<ExprNode>> idx)
      : array_expr(std::move(arr)), indices(std::move(idx)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class RecordAccessNode : public ExprNode {
 public:
  std::unique_ptr<ExprNode> record_expr;
  lexer::Token field;

  RecordAccessNode(std::unique_ptr<ExprNode> rec, lexer::Token f)
      : record_expr(std::move(rec)), field(f) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

}  // namespace ast
