#pragma once

namespace ast {

class AstNode;

// Forward declarations for concrete nodes
class ProgramNode;
class BlockNode;
class VarDeclNode;
class TypeDeclNode;
class ProcDeclNode;
class FuncDeclNode;

// Expressions
class BinOpNode;
class UnaryOpNode;
class NumberNode;
class StringNode;
class IdentNode;
class FuncCallNode;
class ArrayAccessNode;
class RecordAccessNode;

// Statements
class AssignNode;
class IfNode;
class WhileNode;
class RepeatNode;
class ForNode;
class ProcCallNode;
class CompoundStmtNode;

class ASTVisitor {
 public:
  virtual ~ASTVisitor() = default;

  virtual void visit(ProgramNode &node) = 0;
  virtual void visit(BlockNode &node) = 0;
  virtual void visit(VarDeclNode &node) = 0;
  virtual void visit(TypeDeclNode &node) = 0;
  virtual void visit(ProcDeclNode &node) = 0;
  virtual void visit(FuncDeclNode &node) = 0;

  virtual void visit(BinOpNode &node) = 0;
  virtual void visit(UnaryOpNode &node) = 0;
  virtual void visit(NumberNode &node) = 0;
  virtual void visit(StringNode &node) = 0;
  virtual void visit(IdentNode &node) = 0;
  virtual void visit(FuncCallNode &node) = 0;
  virtual void visit(ArrayAccessNode &node) = 0;
  virtual void visit(RecordAccessNode &node) = 0;

  virtual void visit(AssignNode &node) = 0;
  virtual void visit(IfNode &node) = 0;
  virtual void visit(WhileNode &node) = 0;
  virtual void visit(RepeatNode &node) = 0;
  virtual void visit(ForNode &node) = 0;
  virtual void visit(ProcCallNode &node) = 0;
  virtual void visit(CompoundStmtNode &node) = 0;
};

}  // namespace ast
