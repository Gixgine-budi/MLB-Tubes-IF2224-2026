#include "semantic/semantic_analyzer.hpp"

#include <iostream>
#include <string>

#include "lexer/token.hpp"
#include "semantic/symtable_entries.hpp"

namespace semantic {

namespace {

std::string lower(std::string s) {
  for (char& c : s) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return s;
}

bool isRelational(lexer::TokenType type) {
  using lexer::TokenType;
  return type == TokenType::EQL || type == TokenType::NEQ ||
         type == TokenType::GTR || type == TokenType::GEQ ||
         type == TokenType::LSS || type == TokenType::LEQ;
}

bool isArithmetic(lexer::TokenType type) {
  using lexer::TokenType;
  return type == TokenType::PLUS || type == TokenType::MINUS ||
         type == TokenType::TIMES || type == TokenType::RDIV ||
         type == TokenType::IDIV || type == TokenType::IMOD;
}

}  // namespace

void SemanticAnalyzer::analyze(ast::AstNode& root) { root.accept(*this); }

void SemanticAnalyzer::reportError(const std::string& message) {
  std::cerr << "semantic error: " << message << '\n';
  has_errors_ = true;
}

void SemanticAnalyzer::enterScope() { sym_table.pushBlock(); }

int SemanticAnalyzer::get_base_type(const std::string& type_name) {
  auto entry = sym_table.lookup(type_name);
  return entry ? entry->idx : 0;
}

bool SemanticAnalyzer::isAssignmentCompatible(int target_type, int expr_type) {
  if (target_type == expr_type) return true;  // Aturan 1: Type yang sama

  int real_type = get_base_type("real");
  int int_type = get_base_type("integer");

  if (target_type == real_type && expr_type == int_type) {
    return true;
  }

  return false;
}
}  // namespace semantic
