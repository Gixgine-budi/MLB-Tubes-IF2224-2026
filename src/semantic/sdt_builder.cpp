#include "semantic/sdt_builder.hpp"

#include <cstddef>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

using lexer::TokenType;
using parser::NodeType;

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
