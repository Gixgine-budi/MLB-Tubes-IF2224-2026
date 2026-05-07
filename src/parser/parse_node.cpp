
#include "parser/parse_node.hpp"

#include <iostream>
#include <ostream>

#include "lexer/token.hpp"

namespace parser {

std::ostream& operator<<(std::ostream& os, const ParseNode& node) {
  switch (node.type_) {
    case NodeType::Program:
      return os << "Program";
    case NodeType::ProgramHeader:
      return os << "ProgramHeader";
    case NodeType::DeclarationPart:
      return os << "DeclarationPart";
    case NodeType::ConstDeclaration:
      return os << "ConstDeclaration";
    case NodeType::Constant:
      return os << "Constant";
    case NodeType::TypeDeclaration:
      return os << "TypeDeclaration";
    case NodeType::VarDeclaration:
      return os << "VarDeclaration";
    case NodeType::IdentifierList:
      return os << "IdentifierList";
    case NodeType::Type:
      return os << "Type";
    case NodeType::ArrayType:
      return os << "ArrayType";
    case NodeType::Range:
      return os << "Range";
    case NodeType::Enumerated:
      return os << "Enumerated";
    case NodeType::RecordType:
      return os << "RecordType";
    case NodeType::FieldList:
      return os << "FieldList";
    case NodeType::FieldPart:
      return os << "FieldPart";
    case NodeType::SubprogramDeclaration:
      return os << "SubprogramDeclaration";
    case NodeType::ProcedureDeclaration:
      return os << "ProcedureDeclaration";
    case NodeType::FunctionDeclaration:
      return os << "FunctionDeclaration";
    case NodeType::Block:
      return os << "Block";
    case NodeType::FormalParameterList:
      return os << "FormalParameterList";
    case NodeType::ParameterGroup:
      return os << "ParameterGroup";
    case NodeType::CompoundStatement:
      return os << "CompoundStatement";
    case NodeType::StatementList:
      return os << "StatementList";
    case NodeType::Statement:
      return os << "Statement";
    case NodeType::Variable:
      return os << "Variable";
    case NodeType::ComponentVariable:
      return os << "ComponentVariable";
    case NodeType::IndexList:
      return os << "IndexList";
    case NodeType::AssignmentStatement:
      return os << "AssignmentStatement";
    case NodeType::IfStatement:
      return os << "IfStatement";
    case NodeType::CaseStatement:
      return os << "CaseStatement";
    case NodeType::CaseBlock:
      return os << "CaseBlock";
    case NodeType::WhileStatement:
      return os << "WhileStatement";
    case NodeType::RepeatStatement:
      return os << "RepeatStatement";
    case NodeType::ForStatement:
      return os << "ForStatement";
    case NodeType::FunctionCall:
      return os << "FunctionCall";
    case NodeType::ParameterList:
      return os << "ParameterList";
    case NodeType::Expression:
      return os << "Expression";
    case NodeType::SimpleExpression:
      return os << "SimpleExpression";
    case NodeType::Term:
      return os << "Term";
    case NodeType::Factor:
      return os << "Factor";
    case NodeType::RelationalOperator:
      return os << "RelationalOperator";
    case NodeType::AdditiveOperator:
      return os << "AdditiveOperator";
    case NodeType::MultiplicativeOperator:
      return os << "MultiplicativeOperator";
    case NodeType::TokenNode:
      if (node.token_)
        return os << *node.token_;
      else
        return os << "TokenNode";
  }
  return os;
}

void ParseNode::print() const { printRecursive("", false); }

void ParseNode::printRecursive(const std::string& prefix, bool isLast) const {
  std::cout << prefix << (isLast ? "└─ " : "├─ ") << *this << "\n";

  const std::string child_prefix = prefix + (isLast ? "  " : "│ ");
  for (size_t i = 0; i < children_.size(); ++i) {
    children_[i]->printRecursive(child_prefix, i == children_.size() - 1);
  }
}

}  // namespace parser