#pragma once

#include <memory>
#include <vector>

#include "ast/ast_node.hpp"
#include "lexer/token.hpp"

namespace ast {

class TypeSpecNode : public AstNode {
 public:
  virtual ~TypeSpecNode() = default;
};

class SimpleTypeSpecNode : public TypeSpecNode {
 public:
  lexer::Token name;

  explicit SimpleTypeSpecNode(lexer::Token n) : name(n) {}

  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class SubrangeTypeSpecNode : public TypeSpecNode {
 public:
  std::unique_ptr<AstNode> low;
  std::unique_ptr<AstNode> high;

  SubrangeTypeSpecNode(std::unique_ptr<AstNode> l, std::unique_ptr<AstNode> h)
      : low(std::move(l)), high(std::move(h)) {}

  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayTypeSpecNode : public TypeSpecNode {
 public:
  std::unique_ptr<TypeSpecNode> index_type;
  std::unique_ptr<TypeSpecNode> element_type;

  ArrayTypeSpecNode(std::unique_ptr<TypeSpecNode> index,
                    std::unique_ptr<TypeSpecNode> element)
      : index_type(std::move(index)), element_type(std::move(element)) {}

  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class RecordTypeSpecNode : public TypeSpecNode {
 public:
  std::vector<
      std::pair<std::vector<lexer::Token>, std::unique_ptr<TypeSpecNode>>>
      fields;

  explicit RecordTypeSpecNode(
      std::vector<
          std::pair<std::vector<lexer::Token>, std::unique_ptr<TypeSpecNode>>>
          f)
      : fields(std::move(f)) {}

  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class EnumTypeSpecNode : public TypeSpecNode {
 public:
  std::vector<lexer::Token> literals;

  explicit EnumTypeSpecNode(std::vector<lexer::Token> enum_literals)
      : literals(std::move(enum_literals)) {}

  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

}  // namespace ast