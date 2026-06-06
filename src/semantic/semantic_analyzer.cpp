#include "semantic/semantic_analyzer.hpp"

#include <algorithm>
#include <vector>

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

  int actual_target = target_type;
  while (actual_target >= RESERVED) {
    const auto& t_entry = sym_table.getTabEntry(actual_target);
    if (t_entry.type == static_cast<int>(BuiltinType::Subrange)) {
      actual_target = t_entry.ref; // Found a subrange, extract base type
      break;
    } else if (t_entry.obj == ObjClass::Type && t_entry.type >= RESERVED && t_entry.type != actual_target) {
      actual_target = t_entry.type; 
    } else {
      break; 
    }
  }

  int actual_expr = expr_type;
  while (actual_expr >= RESERVED) {
    const auto& e_entry = sym_table.getTabEntry(actual_expr);
    if (e_entry.type == static_cast<int>(BuiltinType::Subrange)) {
      actual_expr = e_entry.ref; 
      break;
    } else if (e_entry.obj == ObjClass::Type && e_entry.type >= RESERVED && e_entry.type != actual_expr) {
      actual_expr = e_entry.type; 
    } else {
      break; 
    }
  }

  if (actual_target == actual_expr && actual_target != 0) return true;

  if (actual_target == get_base_type("real") &&
      actual_expr == get_base_type("integer")) {
    return true;
  }

  return false;
}

void SemanticAnalyzer::finalizeParamAddresses(int block_idx) {
  const auto& block = sym_table.getBtabEntry(block_idx);
  std::vector<int> param_indices;
  int link = block.lpar;
  while (link >= RESERVED) {
    param_indices.push_back(link);
    link = sym_table.getTabEntry(link).link;
  }
  if (param_indices.empty()) {
    return;
  }

  std::reverse(param_indices.begin(), param_indices.end());
  const int argc = static_cast<int>(param_indices.size());
  for (int i = 0; i < argc; ++i) {
    auto& entry = sym_table.getTabEntry(param_indices[static_cast<std::size_t>(i)]);
    entry.adr = -argc + i;
  }
}

int SemanticAnalyzer::countFormalParams(int proc_or_func_idx) const {
  if (proc_or_func_idx < RESERVED) {
    return 0;
  }
  const auto& entry = sym_table.getTabEntry(proc_or_func_idx);
  if (entry.ref <= 0) {
    return 0;
  }
  int count = 0;
  int link = sym_table.getBtabEntry(entry.ref).lpar;
  while (link >= RESERVED) {
    ++count;
    link = sym_table.getTabEntry(link).link;
  }
  return count;
}

}  // namespace semantic
