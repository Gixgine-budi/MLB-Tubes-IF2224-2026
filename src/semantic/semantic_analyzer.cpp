#include "semantic/semantic_analyzer.hpp"

#include "diagnoser/diagnostic.hpp"
#include "lexer/token.hpp"
#include "semantic/symtable_entries.hpp"

namespace semantic {

SemanticAnalyzer::SemanticAnalyzer(const parser::ParseNode& parse_root,
                                   diag::Diagnoser& diagnoser)
    : sdt_builder_(parse_root, diagnoser), diagnoser_(diagnoser) {}

void SemanticAnalyzer::analyze() {
  sdt_builder_.build();
  if (sdt_builder_.hasErrors()) return;
  sdt_builder_.getAst().accept(*this);
}

const ast::AstNode& SemanticAnalyzer::getAst() const {
  return sdt_builder_.getAst();
}

ast::AstNode& SemanticAnalyzer::getAst() {
  return sdt_builder_.getAst();
}

void SemanticAnalyzer::reportError(const std::string& message,
                                   const lexer::Token* token) {
  diag::Source source{};
  if (token != nullptr) {
    source.line = token->line_num;
    source.col = token->col_num;
    source.length = static_cast<int>(
        std::max<std::size_t>(1, token->lexeme.size()));
  }
  diagnoser_.report({diag::Phase::SEMANTIC, diag::Level::ERROR, source,
                     message, ""});
  has_errors_ = true;
}

void SemanticAnalyzer::enterScope() { sym_table.pushBlock(); }
void SemanticAnalyzer::leaveScope() { sym_table.popBlock(); }

int SemanticAnalyzer::get_base_type(const std::string& type_name) {
  auto entry = sym_table.lookup(type_name);
  return entry ? entry->idx : 0;
}

bool SemanticAnalyzer::isAssignmentCompatible(int target_type, int expr_type) {
  if (target_type == expr_type) return true;
  if (target_type == get_base_type("real") &&
      expr_type == get_base_type("integer")) {
    return true;
  }
  return false;
}

}  // namespace semantic
