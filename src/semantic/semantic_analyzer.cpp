#include "semantic/semantic_analyzer.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "ast/decl_nodes.hpp"
#include "ast/expr_nodes.hpp"
#include "ast/stmt_nodes.hpp"
#include "ast/type_nodes.hpp"
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

ast::AstNode& SemanticAnalyzer::getAst() { return sdt_builder_.getAst(); }

void SemanticAnalyzer::reportError(const std::string& message,
                                   const lexer::Token* token) {
  diag::Source source{};
  if (token != nullptr) {
    source.line = token->line_num;
    source.col = token->col_num;
    source.length =
        static_cast<int>(std::max<std::size_t>(1, token->lexeme.size()));
  }
  diagnoser_.report(
      {diag::Phase::SEMANTIC, diag::Level::ERROR, source, message, ""});
  has_errors_ = true;
}

const lexer::Token* SemanticAnalyzer::sourceToken(
    const ast::AstNode& node) const {
  if (node.token.has_value()) {
    return &*node.token;
  }

  if (const auto* n = dynamic_cast<const ast::ProgramNode*>(&node)) {
    return &n->identifier;
  }
  if (const auto* n = dynamic_cast<const ast::ConstDeclNode*>(&node)) {
    return &n->identifier;
  }
  if (const auto* n = dynamic_cast<const ast::VarDeclNode*>(&node)) {
    return n->identifiers.empty() ? nullptr : &n->identifiers.front();
  }
  if (const auto* n = dynamic_cast<const ast::TypeDeclNode*>(&node)) {
    return &n->identifier;
  }
  if (const auto* n = dynamic_cast<const ast::ProcDeclNode*>(&node)) {
    return &n->identifier;
  }
  if (const auto* n = dynamic_cast<const ast::FuncDeclNode*>(&node)) {
    return &n->identifier;
  }
  if (const auto* n = dynamic_cast<const ast::ParameterNode*>(&node)) {
    return n->identifiers.empty() ? nullptr : &n->identifiers.front();
  }
  if (const auto* n = dynamic_cast<const ast::SimpleTypeSpecNode*>(&node)) {
    return &n->name;
  }
  if (const auto* n = dynamic_cast<const ast::SubrangeTypeSpecNode*>(&node)) {
    if (n->low != nullptr) return sourceToken(*n->low);
    if (n->high != nullptr) return sourceToken(*n->high);
    return nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::ArrayTypeSpecNode*>(&node)) {
    if (n->index_type != nullptr) return sourceToken(*n->index_type);
    if (n->element_type != nullptr) return sourceToken(*n->element_type);
    return nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::RecordTypeSpecNode*>(&node)) {
    for (const auto& field : n->fields) {
      if (!field.first.empty()) return &field.first.front();
    }
    return nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::EnumTypeSpecNode*>(&node)) {
    return n->literals.empty() ? nullptr : &n->literals.front();
  }
  if (const auto* n = dynamic_cast<const ast::BinOpNode*>(&node)) {
    return &n->op;
  }
  if (const auto* n = dynamic_cast<const ast::UnaryOpNode*>(&node)) {
    return &n->op;
  }
  if (const auto* n = dynamic_cast<const ast::NumberNode*>(&node)) {
    return &n->val;
  }
  if (const auto* n = dynamic_cast<const ast::StringNode*>(&node)) {
    return &n->val;
  }
  if (const auto* n = dynamic_cast<const ast::IdentNode*>(&node)) {
    return &n->id;
  }
  if (const auto* n = dynamic_cast<const ast::FuncCallNode*>(&node)) {
    return &n->id;
  }
  if (const auto* n = dynamic_cast<const ast::ArrayAccessNode*>(&node)) {
    if (!n->indices.empty() && n->indices.front() != nullptr) {
      return sourceToken(*n->indices.front());
    }
    if (n->array_expr != nullptr) return sourceToken(*n->array_expr);
    return nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::RecordAccessNode*>(&node)) {
    return &n->field;
  }
  if (const auto* n = dynamic_cast<const ast::AssignNode*>(&node)) {
    if (n->target != nullptr) return sourceToken(*n->target);
    if (n->expr != nullptr) return sourceToken(*n->expr);
    return nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::IfNode*>(&node)) {
    return n->condition != nullptr ? sourceToken(*n->condition) : nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::WhileNode*>(&node)) {
    return n->condition != nullptr ? sourceToken(*n->condition) : nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::RepeatNode*>(&node)) {
    return n->condition != nullptr ? sourceToken(*n->condition) : nullptr;
  }
  if (const auto* n = dynamic_cast<const ast::ForNode*>(&node)) {
    return &n->iterator;
  }
  if (const auto* n = dynamic_cast<const ast::ProcCallNode*>(&node)) {
    return &n->id;
  }

  return nullptr;
}

void SemanticAnalyzer::enterScope() { sym_table.pushBlock(); }
void SemanticAnalyzer::leaveScope() { sym_table.popBlock(); }

int SemanticAnalyzer::get_base_type(const std::string& type_name) const {
  auto entry = sym_table.lookup(type_name);
  return entry ? entry->idx : 0;
}

int SemanticAnalyzer::canonicalType(int type) const {
  int current = type;
  while (current >= RESERVED) {
    const auto& entry = sym_table.getTabEntry(current);
    if (entry.obj == ObjClass::Type && entry.type >= RESERVED &&
        entry.type != current) {
      current = entry.type;
      continue;
    }
    break;
  }
  return current;
}

bool SemanticAnalyzer::isSubrangeType(int type) const {
  const int canonical = canonicalType(type);
  return canonical >= RESERVED && sym_table.getTabEntry(canonical).type ==
                                      static_cast<int>(BuiltinType::Subrange);
}

bool SemanticAnalyzer::isEnumeratedType(int type) const {
  const int canonical = canonicalType(type);
  return canonical >= RESERVED && sym_table.getTabEntry(canonical).type ==
                                      static_cast<int>(BuiltinType::Enumerated);
}

int SemanticAnalyzer::scalarBaseType(int type) const {
  const int canonical = canonicalType(type);
  if (canonical >= RESERVED) {
    const auto& entry = sym_table.getTabEntry(canonical);
    if (entry.type == static_cast<int>(BuiltinType::Subrange) &&
        entry.ref > 0) {
      return scalarBaseType(sym_table.getAtabEntry(entry.ref).xtyp);
    }
  }
  return canonical;
}

bool SemanticAnalyzer::isBooleanLike(int type) const {
  return scalarBaseType(type) == get_base_type("boolean");
}

bool SemanticAnalyzer::isOrdinalLike(int type) const {
  const int base = scalarBaseType(type);
  return base == get_base_type("integer") || base == get_base_type("char") ||
         base == get_base_type("boolean") || isEnumeratedType(base);
}

bool SemanticAnalyzer::isRelationalCompatible(int lhs_type,
                                              int rhs_type) const {
  const int lhs_base = scalarBaseType(lhs_type);
  const int rhs_base = scalarBaseType(rhs_type);
  if (lhs_base == 0 || rhs_base == 0) return false;
  if ((lhs_base == get_base_type("integer") ||
       lhs_base == get_base_type("real")) &&
      (rhs_base == get_base_type("integer") ||
       rhs_base == get_base_type("real"))) {
    return true;
  }
  return lhs_base == rhs_base && isOrdinalLike(lhs_base);
}

std::optional<SemanticAnalyzer::ConstantValue> SemanticAnalyzer::constantValue(
    const ast::AstNode& node) const {
  if (const auto* num = dynamic_cast<const ast::NumberNode*>(&node)) {
    try {
      if (num->is_real) {
        return ConstantValue{get_base_type("real"),
                             static_cast<int>(std::stod(num->val.lexeme))};
      }
      return ConstantValue{get_base_type("integer"),
                           std::stoi(num->val.lexeme)};
    } catch (...) {
      return std::nullopt;
    }
  }

  if (const auto* str = dynamic_cast<const ast::StringNode*>(&node)) {
    if (str->val.type == lexer::TokenType::CHARCON &&
        str->val.lexeme.size() >= 3) {
      return ConstantValue{get_base_type("char"),
                           static_cast<unsigned char>(str->val.lexeme[1])};
    }
    return std::nullopt;
  }

  if (const auto* id = dynamic_cast<const ast::IdentNode*>(&node)) {
    if (auto entry = sym_table.lookup(id->id.lexeme);
        entry && entry->obj == ObjClass::Constant) {
      return ConstantValue{entry->type, entry->adr};
    }
    return std::nullopt;
  }

  if (const auto* unary = dynamic_cast<const ast::UnaryOpNode*>(&node)) {
    if (unary->expr == nullptr) return std::nullopt;
    auto value = constantValue(*unary->expr);
    if (!value) return std::nullopt;
    if (unary->op.type == lexer::TokenType::MINUS) {
      value->value = -value->value;
      return value;
    }
    if (unary->op.type == lexer::TokenType::PLUS) {
      return value;
    }
  }

  return std::nullopt;
}

bool SemanticAnalyzer::checkSubrangeValue(int target_type, int value,
                                          const std::string& context,
                                          const lexer::Token* token) {
  const int target = canonicalType(target_type);
  if (!isSubrangeType(target)) return true;

  const auto& type_entry = sym_table.getTabEntry(target);
  const auto& bounds = sym_table.getAtabEntry(type_entry.ref);
  if (value < bounds.low || value > bounds.high) {
    reportError(context + " is outside subrange bounds", token);
    return false;
  }
  return true;
}

void SemanticAnalyzer::checkSubrangeAssignment(int target_type,
                                               const ast::AstNode& expr,
                                               const std::string& context) {
  if (!isSubrangeType(target_type)) return;
  auto value = constantValue(expr);
  if (value) {
    checkSubrangeValue(target_type, value->value, context, sourceToken(expr));
  }
}

bool SemanticAnalyzer::isAssignmentCompatible(int target_type, int expr_type) {
  const int target = canonicalType(target_type);
  const int expr = canonicalType(expr_type);
  if (target == 0 || expr == 0) return false;
  if (target == expr) return true;

  const int target_base = scalarBaseType(target);
  const int expr_base = scalarBaseType(expr);

  if (target_base == expr_base && target_base != 0 &&
      (isSubrangeType(target) || isSubrangeType(expr))) {
    return true;
  }

  if (target_base == get_base_type("real") &&
      expr_base == get_base_type("integer")) {
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
    auto& entry =
        sym_table.getTabEntry(param_indices[static_cast<std::size_t>(i)]);
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
