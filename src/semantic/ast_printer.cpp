#include "semantic/ast_printer.hpp"

#include <ostream>
#include <string>
#include <vector>

namespace semantic {

ASTPrinter::ASTPrinter(std::ostream &os, bool ascii) : os_(os), ascii_(ascii) {}

const char *ASTPrinter::branch() const { return ascii_ ? "+-- " : "├── "; }
const char *ASTPrinter::lastBranch() const { return ascii_ ? "\\-- " : "└── "; }
const char *ASTPrinter::pipe() const { return ascii_ ? "|   " : "│   "; }
const char *ASTPrinter::blank() const { return "    "; }
const char *ASTPrinter::arrow() const { return ascii_ ? "-> " : "→ "; }

void ASTPrinter::printHead(const std::string &label, const std::string &ann) {
  if (is_root_) {
    is_root_ = false;
    os_ << label;
  } else {
    os_ << prefix_ << (is_last_ ? lastBranch() : branch()) << label;
  }
  if (!ann.empty()) os_ << "  " << arrow() << ann;
  os_ << '\n';
}

std::string ASTPrinter::nodeAnnotation(const ast::AstNode &n) {
  std::string s;
  if (n.expression_type != 0) s += "type:" + std::to_string(n.expression_type);
  if (n.tab_index != 0) {
    if (!s.empty()) s += ", ";
    s += "tab:" + std::to_string(n.tab_index);
  }
  return s;
}

std::string ASTPrinter::enterChildren(bool wasRoot, bool wasLast) {
  std::string saved = prefix_;
  if (!wasRoot) {
    prefix_ += wasLast ? blank() : pipe();
  }
  return saved;
}

void ASTPrinter::leaveChildren(const std::string &saved) { prefix_ = saved; }

void ASTPrinter::visitChild(ast::AstNode &child, bool is_last) {
  is_last_ = is_last;
  child.accept(*this);
}

template <typename PtrVec>
void ASTPrinter::visitChildren(const PtrVec &vec) {
  for (std::size_t i = 0; i < vec.size(); ++i) {
    visitChild(*vec[i], i + 1 == vec.size());
  }
}

void ASTPrinter::print(ast::AstNode &root) {
  is_root_ = true;
  is_last_ = true;
  prefix_.clear();
  root.accept(*this);
}

// ----------------------------- program / block -----------------------------

void ASTPrinter::visit(ast::ProgramNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("ProgramNode('" + node.identifier.lexeme + "')",
            nodeAnnotation(node));
  if (!node.block) return;

  const auto &decls = node.block->declarations;
  const bool has_stmt = node.block->compound_stmt != nullptr;
  const std::size_t n = decls.size() + (has_stmt ? 1 : 0);
  if (n == 0) return;

  const std::string saved = enterChildren(wr, wl);
  std::size_t i = 0;
  for (auto &d : decls) visitChild(*d, ++i == n);
  if (has_stmt) visitChild(*node.block->compound_stmt, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::BlockNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("Block", nodeAnnotation(node));

  const auto &decls = node.declarations;
  const bool has_stmt = node.compound_stmt != nullptr;
  const std::size_t n = decls.size() + (has_stmt ? 1 : 0);
  if (n == 0) return;

  const std::string saved = enterChildren(wr, wl);
  std::size_t i = 0;
  for (auto &d : decls) visitChild(*d, ++i == n);
  if (has_stmt) visitChild(*node.compound_stmt, true);
  leaveChildren(saved);
}

// --------------------------- type specifications ---------------------------

void ASTPrinter::visit(ast::SimpleTypeSpecNode &node) {
  printHead("TypeSpec('" + node.name.lexeme + "')", nodeAnnotation(node));
}

void ASTPrinter::visit(ast::SubrangeTypeSpecNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("SubrangeType", nodeAnnotation(node));
  if (node.low && node.high) {
    const std::string saved = enterChildren(wr, wl);
    visitChild(*node.low, false);
    visitChild(*node.high, true);
    leaveChildren(saved);
  }
}

void ASTPrinter::visit(ast::ArrayTypeSpecNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("ArrayType", nodeAnnotation(node));
  const bool has_idx = node.index_type != nullptr;
  const bool has_elem = node.element_type != nullptr;
  if (!has_idx && !has_elem) return;
  const std::string saved = enterChildren(wr, wl);
  if (has_idx) visitChild(*node.index_type, !has_elem);
  if (has_elem) visitChild(*node.element_type, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::RecordTypeSpecNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("RecordType(" + std::to_string(node.fields.size()) + " fields)",
            nodeAnnotation(node));
  if (node.fields.empty()) return;
  const std::string saved = enterChildren(wr, wl);
  for (std::size_t i = 0; i < node.fields.size(); ++i) {
    const bool last = (i + 1 == node.fields.size());
    std::string names;
    for (const auto &tok : node.fields[i].first) {
      if (!names.empty()) names += ", ";
      names += tok.lexeme;
    }
    is_last_ = last;
    // Print field group inline
    const bool sr = is_root_, sl = is_last_;
    printHead("Field(" + names + ")", nodeAnnotation(node));
    if (node.fields[i].second) {
      const std::string fs = enterChildren(sr, sl);
      visitChild(*node.fields[i].second, true);
      leaveChildren(fs);
    }
  }
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::EnumTypeSpecNode &node) {
  std::string lits;
  for (const auto &tok : node.literals) {
    if (!lits.empty()) lits += ", ";
    lits += tok.lexeme;
  }
  printHead("EnumType(" + lits + ")", nodeAnnotation(node));
}

// ------------------------------- declarations -------------------------------

void ASTPrinter::visit(ast::ConstDeclNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("ConstDecl('" + node.identifier.lexeme + "')",
            nodeAnnotation(node));
  if (node.value) {
    const std::string saved = enterChildren(wr, wl);
    visitChild(*node.value, true);
    leaveChildren(saved);
  }
}

void ASTPrinter::visit(ast::VarDeclNode &node) {
  std::string names;
  for (const auto &tok : node.identifiers) {
    if (!names.empty()) names += ", ";
    names += tok.lexeme;
  }
  printHead("VarDecl(" + names + ")", nodeAnnotation(node));
}

void ASTPrinter::visit(ast::TypeDeclNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("TypeDecl('" + node.identifier.lexeme + "')", nodeAnnotation(node));
  if (node.type_def) {
    const std::string saved = enterChildren(wr, wl);
    visitChild(*node.type_def, true);
    leaveChildren(saved);
  }
}

void ASTPrinter::visit(ast::ProcDeclNode &node) {
  std::string ann = nodeAnnotation(node);
  if (!node.parameters.empty()) {
    if (!ann.empty()) ann += ", ";
    ann += "params:" + std::to_string(node.parameters.size());
  }
  const bool wr = is_root_, wl = is_last_;
  printHead("ProcDecl('" + node.identifier.lexeme + "')", ann);
  if (!node.block) return;

  const auto &decls = node.block->declarations;
  const bool has_stmt = node.block->compound_stmt != nullptr;
  const std::size_t n = decls.size() + (has_stmt ? 1 : 0);
  if (n == 0) return;

  const std::string saved = enterChildren(wr, wl);
  std::size_t i = 0;
  for (auto &d : decls) visitChild(*d, ++i == n);
  if (has_stmt) visitChild(*node.block->compound_stmt, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::FuncDeclNode &node) {
  std::string ann = nodeAnnotation(node);
  if (!node.parameters.empty()) {
    if (!ann.empty()) ann += ", ";
    ann += "params:" + std::to_string(node.parameters.size());
  }
  const bool wr = is_root_, wl = is_last_;
  printHead("FuncDecl('" + node.identifier.lexeme + "')", ann);
  if (!node.block) return;

  const auto &decls = node.block->declarations;
  const bool has_stmt = node.block->compound_stmt != nullptr;
  const std::size_t n = decls.size() + (has_stmt ? 1 : 0);
  if (n == 0) return;

  const std::string saved = enterChildren(wr, wl);
  std::size_t i = 0;
  for (auto &d : decls) visitChild(*d, ++i == n);
  if (has_stmt) visitChild(*node.block->compound_stmt, true);
  leaveChildren(saved);
}

// ------------------------------- expressions -------------------------------

void ASTPrinter::visit(ast::BinOpNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("BinOp('" + node.op.lexeme + "')", nodeAnnotation(node));
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.left, false);
  visitChild(*node.right, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::UnaryOpNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("UnaryOp('" + node.op.lexeme + "')", nodeAnnotation(node));
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.expr, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::NumberNode &node) {
  printHead((node.is_real ? "Real(" : "Int(") + node.val.lexeme + ")",
            nodeAnnotation(node));
}

void ASTPrinter::visit(ast::StringNode &node) {
  const bool is_char = node.val.type == lexer::TokenType::CHARCON;
  printHead((is_char ? "Char(" : "Str(") + node.val.lexeme + ")",
            nodeAnnotation(node));
}

void ASTPrinter::visit(ast::IdentNode &node) {
  printHead("Ident('" + node.id.lexeme + "')", nodeAnnotation(node));
}

void ASTPrinter::visit(ast::FuncCallNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("FuncCall('" + node.id.lexeme + "')", nodeAnnotation(node));
  if (node.args.empty()) return;
  const std::string saved = enterChildren(wr, wl);
  visitChildren(node.args);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::ArrayAccessNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("ArrayAccess", nodeAnnotation(node));
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.array_expr, node.indices.empty());
  visitChildren(node.indices);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::RecordAccessNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("RecordAccess(." + node.field.lexeme + ")", nodeAnnotation(node));
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.record_expr, true);
  leaveChildren(saved);
}

// -------------------------------- statements --------------------------------

void ASTPrinter::visit(ast::AssignNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("Assign", nodeAnnotation(node));
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.target, false);
  visitChild(*node.expr, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::IfNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("If", nodeAnnotation(node));
  const bool has_else = node.else_branch != nullptr;
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.condition, false);
  visitChild(*node.then_branch, !has_else);
  if (has_else) visitChild(*node.else_branch, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::WhileNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("While", nodeAnnotation(node));
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.condition, false);
  visitChild(*node.body, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::RepeatNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("Repeat", nodeAnnotation(node));
  const std::size_t n = node.statements.size() + 1;
  const std::string saved = enterChildren(wr, wl);
  std::size_t i = 0;
  for (auto &s : node.statements) visitChild(*s, ++i == n);
  visitChild(*node.condition, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::ForNode &node) {
  const bool wr = is_root_, wl = is_last_;
  std::string label = "For('" + node.iterator.lexeme + "' ";
  label += node.is_downto ? "downto" : "to";
  label += ")";
  printHead(label, nodeAnnotation(node));
  const std::string saved = enterChildren(wr, wl);
  visitChild(*node.initial, false);
  visitChild(*node.final, false);
  visitChild(*node.body, true);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::ProcCallNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("ProcCall('" + node.id.lexeme + "')", nodeAnnotation(node));
  if (node.args.empty()) return;
  const std::string saved = enterChildren(wr, wl);
  visitChildren(node.args);
  leaveChildren(saved);
}

void ASTPrinter::visit(ast::CompoundStmtNode &node) {
  const bool wr = is_root_, wl = is_last_;
  printHead("Compound", nodeAnnotation(node));
  if (node.statements.empty()) return;
  const std::string saved = enterChildren(wr, wl);
  visitChildren(node.statements);
  leaveChildren(saved);
}

}  // namespace semantic
