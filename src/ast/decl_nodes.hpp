#pragma once

#include <memory>
#include <vector>

#include "ast/ast_node.hpp"
#include "ast/stmt_nodes.hpp"
#include "ast/type_nodes.hpp"
#include "lexer/token.hpp"

namespace ast {

class VarDeclNode : public AstNode {
 public:
  std::vector<lexer::Token> identifiers;
  std::unique_ptr<TypeSpecNode> type_spec;  // Type identifier, array type, etc.

  VarDeclNode(std::vector<lexer::Token> ids,
              std::unique_ptr<TypeSpecNode> t_spec)
      : identifiers(std::move(ids)), type_spec(std::move(t_spec)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class TypeDeclNode : public AstNode {
 public:
  lexer::Token identifier;
  std::unique_ptr<TypeSpecNode> type_def;

  TypeDeclNode(lexer::Token id, std::unique_ptr<TypeSpecNode> t_def)
      : identifier(id), type_def(std::move(t_def)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Represents formal parameters like: v1, v2 : Integer; var v3 : Boolean
class ParameterNode : public AstNode {
 public:
  std::vector<lexer::Token> identifiers;
  std::unique_ptr<TypeSpecNode> type_spec;
  bool is_var;  // True if passed by reference (var param)

  ParameterNode(std::vector<lexer::Token> ids,
                std::unique_ptr<TypeSpecNode> t_spec, bool var_param)
      : identifiers(std::move(ids)),
        type_spec(std::move(t_spec)),
        is_var(var_param) {}

  // Using a separate accept if it needs one, but usually visited through
  // Proc/Func decl
  void accept(ASTVisitor &) override { /* often handled by parent */
  }
};

class BlockNode : public AstNode {
 public:
  std::vector<std::unique_ptr<AstNode>>
      declarations;  // Const, Type, Var, Proc, Func
  std::unique_ptr<CompoundStmtNode> compound_stmt;

  BlockNode(std::vector<std::unique_ptr<AstNode>> decls,
            std::unique_ptr<CompoundStmtNode> c_stmt)
      : declarations(std::move(decls)), compound_stmt(std::move(c_stmt)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ProcDeclNode : public AstNode {
 public:
  lexer::Token identifier;
  std::vector<std::unique_ptr<ParameterNode>> parameters;
  std::unique_ptr<BlockNode> block;

  ProcDeclNode(lexer::Token id,
               std::vector<std::unique_ptr<ParameterNode>> params,
               std::unique_ptr<BlockNode> b)
      : identifier(id), parameters(std::move(params)), block(std::move(b)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class FuncDeclNode : public AstNode {
 public:
  lexer::Token identifier;
  std::vector<std::unique_ptr<ParameterNode>> parameters;
  std::unique_ptr<TypeSpecNode> return_type;
  std::unique_ptr<BlockNode> block;

  FuncDeclNode(lexer::Token id,
               std::vector<std::unique_ptr<ParameterNode>> params,
               std::unique_ptr<TypeSpecNode> ret_type,
               std::unique_ptr<BlockNode> b)
      : identifier(id),
        parameters(std::move(params)),
        return_type(std::move(ret_type)),
        block(std::move(b)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ProgramNode : public AstNode {
 public:
  lexer::Token identifier;
  std::vector<lexer::Token> program_params;  // e.g., (input, output)
  std::unique_ptr<BlockNode> block;

  ProgramNode(lexer::Token id, std::vector<lexer::Token> params,
              std::unique_ptr<BlockNode> b)
      : identifier(id),
        program_params(std::move(params)),
        block(std::move(b)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

}  // namespace ast
