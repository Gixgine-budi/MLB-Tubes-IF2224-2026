#pragma once

#include <vector>

#include "ast/ast_visitor.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "ast/type_nodes.hpp"
#include "ir/instruction.hpp"
#include "semantic/symbol_table.hpp"

namespace ir {

class CodeGenerator : public ast::ASTVisitor {
 public:
  explicit CodeGenerator(semantic::SymbolTable& st);

  const std::vector<Instruction>& getCode() const;
  void printCode() const;

  void visit(ast::ProgramNode& node) override;
  void visit(ast::BlockNode& node) override;
  void visit(ast::CompoundStmtNode& node) override;
  void visit(ast::AssignNode& node) override;
  void visit(ast::IfNode& node) override;
  void visit(ast::WhileNode& node) override;
  void visit(ast::RepeatNode& node) override;
  void visit(ast::ForNode& node) override;
  void visit(ast::ProcCallNode& node) override;
  void visit(ast::BinOpNode& node) override;
  void visit(ast::NumberNode& node) override;
  void visit(ast::IdentNode& node) override;
  void visit(ast::ConstDeclNode& node) override;
  void visit(ast::VarDeclNode& node) override;
  void visit(ast::TypeDeclNode& node) override;
  void visit(ast::ProcDeclNode& node) override;
  void visit(ast::FuncDeclNode& node) override;
  void visit(ast::UnaryOpNode& node) override;
  void visit(ast::StringNode& node) override;
  void visit(ast::FuncCallNode& node) override;
  void visit(ast::ArrayAccessNode& node) override;
  void visit(ast::RecordAccessNode& node) override;
  void visit(ast::SimpleTypeSpecNode& node) override;
  void visit(ast::SubrangeTypeSpecNode& node) override;
  void visit(ast::ArrayTypeSpecNode& node) override;
  void visit(ast::RecordTypeSpecNode& node) override;
  void visit(ast::EnumTypeSpecNode& node) override;

 private:
  int emit(OpCode op, int l, int a, int aux = 0);
  int mainFrameSize() const;
  bool emitArrayRelativeIndex(ast::ArrayAccessNode& node, int& level_diff,
                              int& base_adr, int& array_size);
  void emitProcOrFuncBody(ast::BlockNode* block, int block_ref);
  void emitRangeCheck(int type_idx);
  int canonicalType(int type_idx) const;
  int subrangeRef(int type_idx) const;
  int arrayElementType(ast::ArrayAccessNode& node) const;
  void emitStore(OpCode op, int l, int a, int type_idx, int aux = 0);

  std::vector<Instruction> code;
  semantic::SymbolTable& sym_table;
  int current_level;
};

}  // namespace ir
