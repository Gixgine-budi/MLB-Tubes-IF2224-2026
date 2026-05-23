#include "semantic/sdt_builder.hpp"

#include <cstddef>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

namespace {

using lexer::TokenType;
using parser::NodeType;

bool is_ignored_declaration_token(TokenType type) {
  return type == TokenType::CONSTSY || type == TokenType::TYPESY ||
         type == TokenType::VARSY || type == TokenType::EQL ||
         type == TokenType::COLON || type == TokenType::SEMICOLON ||
         type == TokenType::COMMA || type == TokenType::ARRAYSY ||
         type == TokenType::LBRACK || type == TokenType::RBRACK ||
         type == TokenType::OFSY || type == TokenType::RECORDSY ||
         type == TokenType::ENDSY || type == TokenType::LPARENT ||
         type == TokenType::RPARENT || type == TokenType::PERIOD;
}

AstPtr named_type_from_token(const lexer::Token& token) {
  auto node = std::make_unique<AstNode>(AstKind::NamedType, token);
  node->setName(token.lexeme);
  return node;
}

}  // namespace

AstNode::AstNode(AstKind kind) : kind_(kind) {}

AstNode::AstNode(AstKind kind, std::string value)
    : kind_(kind), value_(std::move(value)) {}

AstNode::AstNode(AstKind kind, const lexer::Token& token)
    : kind_(kind), value_(token.lexeme), token_(token) {}

AstNode& AstNode::addChild(AstPtr child) {
  if (child) {
    children_.push_back(std::move(child));
  }
  return *this;
}

std::ostream& operator<<(std::ostream& os, AstKind kind) {
  switch (kind) {
    case AstKind::Program:
      return os << "Program";
    case AstKind::ProgramHeader:
      return os << "ProgramHeader";
    case AstKind::Declarations:
      return os << "Declarations";
    case AstKind::ConstDecl:
      return os << "ConstDecl";
    case AstKind::TypeDecl:
      return os << "TypeDecl";
    case AstKind::VarDecl:
      return os << "VarDecl";
    case AstKind::NamedType:
      return os << "NamedType";
    case AstKind::ArrayType:
      return os << "ArrayType";
    case AstKind::RangeType:
      return os << "RangeType";
    case AstKind::EnumType:
      return os << "EnumType";
    case AstKind::RecordType:
      return os << "RecordType";
    case AstKind::FieldDecl:
      return os << "FieldDecl";
    case AstKind::Block:
      return os << "Block";
    case AstKind::CompoundStatement:
      return os << "CompoundStatement";
    case AstKind::StatementList:
      return os << "StatementList";
    case AstKind::Statement:
      return os << "Statement";
    case AstKind::Assign:
      return os << "Assign";
    case AstKind::If:
      return os << "If";
    case AstKind::Case:
      return os << "Case";
    case AstKind::CaseBranch:
      return os << "CaseBranch";
    case AstKind::While:
      return os << "While";
    case AstKind::Repeat:
      return os << "Repeat";
    case AstKind::For:
      return os << "For";
    case AstKind::Call:
      return os << "Call";
    case AstKind::ParameterList:
      return os << "ParameterList";
    case AstKind::VarRef:
      return os << "VarRef";
    case AstKind::ArrayAccess:
      return os << "ArrayAccess";
    case AstKind::FieldAccess:
      return os << "FieldAccess";
    case AstKind::Literal:
      return os << "Literal";
    case AstKind::UnaryOp:
      return os << "UnaryOp";
    case AstKind::BinOp:
      return os << "BinOp";
    case AstKind::Empty:
      return os << "Empty";
    case AstKind::Error:
      return os << "Error";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const AstNode& node) {
  os << node.kind();
  if (!node.name().empty()) {
    os << "(" << node.name() << ")";
  } else if (!node.op().empty()) {
    os << "(" << node.op() << ")";
  } else if (!node.value().empty()) {
    os << "(" << node.value() << ")";
  }

  const auto& annotation = node.annotation();
  if (!annotation.type.empty() || annotation.tab_index >= 0 ||
      annotation.block_index >= 0 || annotation.array_index >= 0 ||
      annotation.lexical_level >= 0) {
    os << " [";
    bool has_previous = false;
    auto write_separator = [&]() {
      if (has_previous) os << ", ";
      has_previous = true;
    };
    if (!annotation.type.empty()) {
      write_separator();
      os << "type=" << annotation.type;
    }
    if (annotation.tab_index >= 0) {
      write_separator();
      os << "tab=" << annotation.tab_index;
    }
    if (annotation.block_index >= 0) {
      write_separator();
      os << "block=" << annotation.block_index;
    }
    if (annotation.array_index >= 0) {
      write_separator();
      os << "array=" << annotation.array_index;
    }
    if (annotation.lexical_level >= 0) {
      write_separator();
      os << "level=" << annotation.lexical_level;
    }
    os << "]";
  }
  return os;
}

void AstNode::print(std::ostream& os, int indent) const {
  for (int i = 0; i < indent; ++i) os << "  ";
  os << *this << '\n';
  for (const auto& child : children_) {
    child->print(os, indent + 1);
  }
}

AstPtr SdtBuilder::build(const parser::ParseNode& node) const {
  switch (node.type()) {
    case NodeType::Program:
      return buildProgram(node);
    case NodeType::ProgramHeader:
      return buildProgramHeader(node);
    case NodeType::DeclarationPart:
      return buildDeclarationPart(node);
    case NodeType::ConstDeclaration:
      return buildConstDeclaration(node);
    case NodeType::TypeDeclaration:
      return buildTypeDeclaration(node);
    case NodeType::VarDeclaration:
      return buildVarDeclaration(node);
    case NodeType::Type:
      return buildType(node);
    case NodeType::ArrayType:
      return buildArrayType(node);
    case NodeType::Range:
      return buildRange(node);
    case NodeType::Enumerated:
      return buildEnumerated(node);
    case NodeType::RecordType:
      return buildRecordType(node);
    case NodeType::FieldList:
      return buildFieldList(node);
    case NodeType::FieldPart:
      return buildFieldPart(node);
    case NodeType::Constant:
      return buildConstant(node);
    case NodeType::Expression:
      return buildExpression(node);
    case NodeType::SimpleExpression:
      return buildSimpleExpression(node);
    case NodeType::Term:
      return buildTerm(node);
    case NodeType::Factor:
      return buildFactor(node);
    case NodeType::Variable:
      return buildVariable(node);
    case NodeType::FunctionCall:
      return buildFunctionCall(node);
    case NodeType::ParameterList:
      return buildParameterList(node);
    case NodeType::Error:
      return makeError();
    default:
      return makeEmpty();
  }
}

AstPtr SdtBuilder::buildProgram(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Program);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::ProgramHeader ||
        child->type() == NodeType::DeclarationPart) {
      ast->addChild(build(*child));
    } else if (child->type() == NodeType::CompoundStatement) {
      ast->addChild(std::make_unique<AstNode>(AstKind::CompoundStatement));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildProgramHeader(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::ProgramHeader);
  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT) {
      ast->setName(token->lexeme);
      break;
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildDeclarationPart(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::ConstDeclaration ||
        child->type() == NodeType::TypeDeclaration ||
        child->type() == NodeType::VarDeclaration) {
      ast->addChild(build(*child));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildConstDeclaration(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto& children = node.children();

  for (std::size_t i = 0; i < children.size(); ++i) {
    const auto* token = tokenOf(*children[i]);
    if (!token || token->type != TokenType::IDENT) continue;

    auto decl = std::make_unique<AstNode>(AstKind::ConstDecl, *token);
    decl->setName(token->lexeme);

    for (std::size_t j = i + 1; j < children.size(); ++j) {
      if (children[j]->type() == NodeType::Constant) {
        decl->addChild(buildConstant(*children[j]));
        i = j;
        break;
      }
      if (const auto* next_token = tokenOf(*children[j]);
          next_token && next_token->type == TokenType::IDENT) {
        break;
      }
    }

    ast->addChild(std::move(decl));
  }

  return ast;
}

AstPtr SdtBuilder::buildTypeDeclaration(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto& children = node.children();

  for (std::size_t i = 0; i < children.size(); ++i) {
    const auto* token = tokenOf(*children[i]);
    if (!token || token->type != TokenType::IDENT) continue;

    auto decl = std::make_unique<AstNode>(AstKind::TypeDecl, *token);
    decl->setName(token->lexeme);

    for (std::size_t j = i + 1; j < children.size(); ++j) {
      if (children[j]->type() == NodeType::Type) {
        decl->addChild(buildType(*children[j]));
        i = j;
        break;
      }
      if (const auto* next_token = tokenOf(*children[j]);
          next_token && next_token->type == TokenType::IDENT) {
        break;
      }
    }

    ast->addChild(std::move(decl));
  }

  return ast;
}

AstPtr SdtBuilder::buildVarDeclaration(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto& children = node.children();

  for (std::size_t i = 0; i < children.size(); ++i) {
    if (children[i]->type() != NodeType::IdentifierList) continue;

    const auto names = identifierList(*children[i]);
    const parser::ParseNode* type_node = nullptr;
    for (std::size_t j = i + 1; j < children.size(); ++j) {
      if (children[j]->type() == NodeType::Type) {
        type_node = children[j].get();
        i = j;
        break;
      }
      if (children[j]->type() == NodeType::IdentifierList) {
        break;
      }
    }

    for (const auto& name : names) {
      auto decl = std::make_unique<AstNode>(AstKind::VarDecl);
      decl->setName(name);
      if (type_node) {
        decl->addChild(buildType(*type_node));
      } else {
        decl->addChild(makeError());
      }
      ast->addChild(std::move(decl));
    }
  }

  return ast;
}

AstPtr SdtBuilder::buildType(const parser::ParseNode& node) const {
  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT) {
      return named_type_from_token(*token);
    }

    switch (child->type()) {
      case NodeType::ArrayType:
        return buildArrayType(*child);
      case NodeType::Range:
        return buildRange(*child);
      case NodeType::Enumerated:
        return buildEnumerated(*child);
      case NodeType::RecordType:
        return buildRecordType(*child);
      case NodeType::Error:
        return makeError();
      default:
        break;
    }
  }
  return makeError();
}

AstPtr SdtBuilder::buildArrayType(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::ArrayType);
  const parser::ParseNode* element_type = nullptr;
  bool in_index = false;

  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child)) {
      if (token->type == TokenType::LBRACK) {
        in_index = true;
      } else if (token->type == TokenType::RBRACK) {
        in_index = false;
      } else if (in_index && token->type == TokenType::IDENT) {
        ast->addChild(named_type_from_token(*token));
      }
      continue;
    }

    if (child->type() == NodeType::Range && in_index) {
      ast->addChild(buildRange(*child));
    } else if (child->type() == NodeType::Type) {
      element_type = child.get();
    }
  }

  if (element_type) {
    ast->addChild(buildType(*element_type));
  } else {
    ast->addChild(makeError());
  }

  return ast;
}

AstPtr SdtBuilder::buildRange(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::RangeType);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::Constant) {
      ast->addChild(buildConstant(*child));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildEnumerated(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::EnumType);
  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT) {
      auto literal = std::make_unique<AstNode>(AstKind::Literal, *token);
      literal->setName(token->lexeme);
      literal->annotation().type = "enumerated";
      ast->addChild(std::move(literal));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildRecordType(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::RecordType);
  if (const auto* fields = firstChildOf(node, NodeType::FieldList)) {
    ast->addChild(buildFieldList(*fields));
  }
  return ast;
}

AstPtr SdtBuilder::buildFieldList(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  for (const auto& child : node.children()) {
    if (child->type() == NodeType::FieldPart) {
      ast->addChild(buildFieldPart(*child));
    }
  }
  return ast;
}

AstPtr SdtBuilder::buildFieldPart(const parser::ParseNode& node) const {
  auto ast = std::make_unique<AstNode>(AstKind::Declarations);
  const auto* ids = firstChildOf(node, NodeType::IdentifierList);
  const auto* type = firstChildOf(node, NodeType::Type);

  if (!ids) return makeError();

  for (const auto& name : identifierList(*ids)) {
    auto field = std::make_unique<AstNode>(AstKind::FieldDecl);
    field->setName(name);
    if (type) {
      field->addChild(buildType(*type));
    } else {
      field->addChild(makeError());
    }
    ast->addChild(std::move(field));
  }

  return ast;
}

AstPtr SdtBuilder::buildConstant(const parser::ParseNode& node) const {
  std::string sign;
  for (const auto& child : node.children()) {
    const auto* token = tokenOf(*child);
    if (!token) continue;

    if (token->type == TokenType::PLUS || token->type == TokenType::MINUS) {
      sign = tokenOperator(*token);
      continue;
    }

    if (is_ignored_declaration_token(token->type)) continue;

    if (token->type == TokenType::IDENT) {
      auto ident = std::make_unique<AstNode>(AstKind::VarRef, *token);
      ident->setName(token->lexeme);
      if (sign == "-") {
        auto unary = std::make_unique<AstNode>(AstKind::UnaryOp);
        unary->setOp("-");
        unary->addChild(std::move(ident));
        return unary;
      }
      return ident;
    }

    auto literal = std::make_unique<AstNode>(AstKind::Literal, *token);
    literal->setValue(sign + token->lexeme);
    literal->annotation().type = literalType(*token);
    return literal;
  }
  return makeError();
}

bool SdtBuilder::isToken(const parser::ParseNode& node) {
  return node.type() == NodeType::TokenNode && node.token().has_value();
}

bool SdtBuilder::isToken(const parser::ParseNode& node, TokenType type) {
  return isToken(node) && node.token()->type == type;
}

const lexer::Token* SdtBuilder::tokenOf(const parser::ParseNode& node) {
  if (!isToken(node)) return nullptr;
  return &*node.token();
}

const parser::ParseNode* SdtBuilder::childAt(const parser::ParseNode& node,
                                             std::size_t index) {
  if (index >= node.children().size()) return nullptr;
  return node.children()[index].get();
}

const parser::ParseNode* SdtBuilder::firstChildOf(
    const parser::ParseNode& node, NodeType type) {
  for (const auto& child : node.children()) {
    if (child->type() == type) return child.get();
  }
  return nullptr;
}

std::vector<std::string> SdtBuilder::identifierList(
    const parser::ParseNode& node) {
  std::vector<std::string> names;
  for (const auto& child : node.children()) {
    if (const auto* token = tokenOf(*child);
        token && token->type == TokenType::IDENT) {
      names.push_back(token->lexeme);
    }
  }
  return names;
}

std::string SdtBuilder::tokenOperator(const lexer::Token& token) {
  switch (token.type) {
    case TokenType::PLUS:
      return "+";
    case TokenType::MINUS:
      return "-";
    case TokenType::TIMES:
      return "*";
    case TokenType::RDIV:
      return "/";
    case TokenType::IDIV:
      return "div";
    case TokenType::IMOD:
      return "mod";
    case TokenType::ANDSY:
      return "and";
    case TokenType::ORSY:
      return "or";
    case TokenType::NOTSY:
      return "not";
    case TokenType::EQL:
      return "=";
    case TokenType::NEQ:
      return "<>";
    case TokenType::GTR:
      return ">";
    case TokenType::GEQ:
      return ">=";
    case TokenType::LSS:
      return "<";
    case TokenType::LEQ:
      return "<=";
    default:
      return std::string(lexer::toString(token.type));
  }
}

std::string SdtBuilder::literalType(const lexer::Token& token) {
  switch (token.type) {
    case TokenType::INTCON:
      return "integer";
    case TokenType::REALCON:
      return "real";
    case TokenType::CHARCON:
      return "char";
    case TokenType::STRING:
      return "string";
    default:
      return "";
  }
}

AstPtr SdtBuilder::makeError() {
  return std::make_unique<AstNode>(AstKind::Error);
}

AstPtr SdtBuilder::makeEmpty() {
  return std::make_unique<AstNode>(AstKind::Empty);
}

}  // namespace semantic
