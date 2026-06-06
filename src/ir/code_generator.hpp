#pragma once

#include <iosfwd>
#include <vector>

#include "ast/ast_visitor.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "ast/type_nodes.hpp"
#include "ir/instruction.hpp"
#include "semantic/symbol_table.hpp"

namespace ir {

/**
 * @brief Lowers a decorated AST into stack-machine intermediate code.
 *
 * Code generation depends on the semantic symbol table for lexical levels,
 * stack addresses, procedure/function metadata, and composite type layout.
 * Generated code is intentionally simple: expressions leave their result on
 * the VM stack, statements consume the values they store or test, and runtime
 * range checks are emitted for assignments into subrange-typed targets.
 */
class CodeGenerator : public ast::ASTVisitor {
 public:
  /**
   * @brief Construct a generator using the completed semantic symbol table.
   *
   * @param st Symbol table populated by SemanticAnalyzer. The generator keeps a
   * borrowed reference and expects it to outlive the generator.
   */
  explicit CodeGenerator(semantic::SymbolTable& st);

  /**
   * @brief Return the generated instruction buffer.
   *
   * @return const std::vector<Instruction>& immutable IR sequence
   */
  const std::vector<Instruction>& getCode() const;

  /**
   * @brief Print the generated IR in address-prefixed textual form.
   *
   * @param out destination stream for the instruction listing
   */
  void print(std::ostream& out) const;

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
  /**
   * @brief Append one instruction and return its code address.
   *
   * @return int zero-based instruction index
   */
  int emit(OpCode op, int l, int a, int aux = 0);

  /**
   * @brief Compute the initial stack frame size for the main program block.
   *
   * Includes static/dynamic/control slots plus global and main-block variable
   * storage as represented in the symbol table.
   */
  int mainFrameSize() const;

  /**
   * @brief Emit code that leaves an array index relative to its lower bound.
   *
   * The target must currently be an identifier-backed array access. On success,
   * the index expression is emitted and normalized so LDX/STX can use a
   * zero-based offset.
   */
  bool emitArrayRelativeIndex(ast::ArrayAccessNode& node, int& level_diff,
                              int& base_adr, int& array_size);

  /**
   * @brief Emit a procedure/function body prologue, body, and return.
   */
  void emitProcOrFuncBody(ast::BlockNode* block, int block_ref);

  /**
   * @brief Emit a VM range check when the target type is a subrange.
   */
  void emitRangeCheck(int type_idx);

  /**
   * @brief Resolve aliases until the underlying symbol table type is reached.
   */
  int canonicalType(int type_idx) const;

  /**
   * @brief Return the bounds-table reference for a subrange type.
   *
   * @return int atab index, or 0 when the type is not a subrange
   */
  int subrangeRef(int type_idx) const;

  /**
   * @brief Resolve the element type of an identifier-backed array access.
   */
  int arrayElementType(ast::ArrayAccessNode& node) const;

  /**
   * @brief Emit optional range checking followed by the actual store opcode.
   */
  void emitStore(OpCode op, int l, int a, int type_idx, int aux = 0);

  std::vector<Instruction> code;
  semantic::SymbolTable& sym_table;
  int current_level;
};

}  // namespace ir
