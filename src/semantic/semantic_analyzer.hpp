#pragma once

#include <optional>

#include "ast/ast_visitor.hpp"
#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "diagnoser/diagnoser.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"
#include "semantic/symbol_table.hpp"

namespace semantic {

class SemanticAnalyzer : public ast::ASTVisitor {
 public:
  /**
   * @brief Construct a new Semantic Analyzer object
   *
   * @param parse_root Parse tree root to analyze (borrowed reference)
   * @param diagnoser Diagnoser to report semantic errors into (borrowed
   * reference)
   */
  SemanticAnalyzer(const parser::ParseNode &parse_root,
                   diag::Diagnoser &diagnoser);
  ~SemanticAnalyzer() override = default;

  /**
   * @brief Build the AST from the parse tree, then decorate it and populate the
   * symbol table. Errors are reported through the bound Diagnoser.
   *
   */
  void analyze();

  /**
   * @brief Get the Ast object (const ref)
   *
   * @return const ast::AstNode&
   */
  const ast::AstNode &getAst() const;

  /**
   * @brief Get the Ast object (non-const) for mutation
   *
   * @return ast::AstNode&
   */
  ast::AstNode &getAst();

  /**
   * @brief Get the Symbol Table object
   *
   * @return const SymbolTable&
   */
  const SymbolTable &getSymbolTable() const { return sym_table; }
  SymbolTable &getSymbolTable() { return sym_table; }

  // --- Visitor Overrides ---

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
  struct ConstantValue {
    int type = 0;
    int value = 0;
  };

  SDTBuilder sdt_builder_;
  SymbolTable sym_table;
  diag::Diagnoser &diagnoser_;
  bool has_errors_ = false;
  int anon_type_counter_ = 0;

  /**
   * @brief Report error to diagnoser with source location from the given parse
   * node's first token.
   *
   * @param message Error message to report.
   * @param token Optional token to extract source location from
   */
  void reportError(const std::string &message,
                   const lexer::Token *token = nullptr);

  /**
   * @brief Get the source token associated with an AST node for error report
   *
   * @param node the AST node to get the source token from
   * @return const lexer::Token* pointer to the source token, or nullptr
   */
  const lexer::Token *sourceToken(const ast::AstNode &node) const;

  /**
   * @brief Resolve the type specification and return its corresponding type ID.
   *
   * @param spec Type specification node to resolve.
   * @return int Type ID corresponding to the resolved type.
   */
  int resolveTypeSpec(ast::TypeSpecNode &spec);

  /**
   * @brief Resolve the type of an expression and return its corresponding type
   * ID.
   *
   * @param name Type name to resolve.
   * @return int Type ID corresponding to the resolved type, or -1 if resolution
   * fails.
   */
  int resolveSimpleTypeName(const std::string &name,
                            const lexer::Token *token = nullptr);

  /**
   * @brief Enter a new scope level in the symbol table.
   *
   */
  void enterScope();

  /**
   * @brief Leave the current scope level in the symbol table, discarding any
   * symbols declared in that scope.
   *
   */
  void leaveScope();

  /**
   * @brief Get the base type ID for a given type name.
   *
   * @param type_name Name of the type.
   * @return int Base type ID corresponding to the type name.
   */
  int get_base_type(const std::string &type_name) const;

  /**
   * @brief Check if an expression of a given type can be assigned to a target
   * of another type.
   *
   * @param target_type Type ID of the assignment target.
   * @param expr_type Type ID of the expression being assigned.
   * @return bool True if the assignment is compatible, false otherwise.
   */
  bool isAssignmentCompatible(int target_type, int expr_type);

  int canonicalType(int type) const;
  int scalarBaseType(int type) const;
  bool isSubrangeType(int type) const;
  bool isEnumeratedType(int type) const;
  bool isBooleanLike(int type) const;
  bool isOrdinalLike(int type) const;
  bool isRelationalCompatible(int lhs_type, int rhs_type) const;
  std::optional<ConstantValue> constantValue(const ast::AstNode &node) const;
  bool checkSubrangeValue(int target_type, int value,
                          const std::string &context,
                          const lexer::Token *token = nullptr);
  void checkSubrangeAssignment(int target_type, const ast::AstNode &expr,
                               const std::string &context);

  /**
   * @brief Create an internal anonymous type entry for composite/derived type
   * specs.
   */
  int makeAnonymousType(int raw_type, int ref = 0);

  /**
   * @brief Best-effort extraction of integer value from a constant AST node.
   */
  int constIntValue(const ast::AstNode *node, int fallback = 0) const;

  int countFormalParams(int proc_or_func_idx) const;

  void finalizeParamAddresses(int block_idx);
};

}  // namespace semantic
