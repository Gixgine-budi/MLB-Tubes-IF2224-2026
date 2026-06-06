#include "ir/code_generator.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

#include "lexer/token.hpp"
#include "semantic/symtable_entries.hpp"

namespace ir {

CodeGenerator::CodeGenerator(semantic::SymbolTable& st)
    : sym_table(st), current_level(0) {}

const std::vector<Instruction>& CodeGenerator::getCode() const { return code; }

int CodeGenerator::emit(OpCode op, int l, int a, int aux) {
  code.push_back({op, l, a, aux});
  return static_cast<int>(code.size()) - 1;
}

int CodeGenerator::mainFrameSize() const {
  int size = sym_table.getBtabEntry(0).vsze;
  if (sym_table.currentLevel() >= 1) {
    size += sym_table.getBtabEntry(1).vsze;
  }
  return 3 + size;
}

int CodeGenerator::canonicalType(int type_idx) const {
  int current = type_idx;
  while (current >= semantic::RESERVED) {
    const auto& entry = sym_table.getTabEntry(current);
    if (entry.obj == semantic::ObjClass::Type &&
        entry.type >= semantic::RESERVED && entry.type != current) {
      current = entry.type;
      continue;
    }
    break;
  }
  return current;
}

int CodeGenerator::subrangeRef(int type_idx) const {
  const int canonical = canonicalType(type_idx);
  if (canonical < semantic::RESERVED) {
    return 0;
  }
  const auto& entry = sym_table.getTabEntry(canonical);
  if (entry.type == static_cast<int>(semantic::BuiltinType::Subrange)) {
    return entry.ref;
  }
  return 0;
}

void CodeGenerator::emitRangeCheck(int type_idx) {
  const int ref = subrangeRef(type_idx);
  if (ref <= 0) {
    return;
  }
  const auto& bounds = sym_table.getAtabEntry(ref);
  emit(OpCode::CHK, bounds.low, bounds.high);
}

int CodeGenerator::arrayElementType(ast::ArrayAccessNode& node) const {
  auto* ident = dynamic_cast<ast::IdentNode*>(node.array_expr.get());
  if (ident == nullptr || ident->tab_index == 0) {
    return 0;
  }
  const auto& var_entry = sym_table.getTabEntry(ident->tab_index);
  const auto& type_entry = sym_table.getTabEntry(canonicalType(var_entry.type));
  if (type_entry.type != static_cast<int>(semantic::BuiltinType::Array) ||
      type_entry.ref <= 0) {
    return 0;
  }
  return sym_table.getAtabEntry(type_entry.ref).etyp;
}

void CodeGenerator::emitStore(OpCode op, int l, int a, int type_idx, int aux) {
  emitRangeCheck(type_idx);
  emit(op, l, a, aux);
}

bool CodeGenerator::emitArrayRelativeIndex(ast::ArrayAccessNode& node,
                                           int& level_diff, int& base_adr,
                                           int& array_size) {
  auto* ident = dynamic_cast<ast::IdentNode*>(node.array_expr.get());
  if (ident == nullptr || ident->tab_index == 0 || node.indices.empty()) {
    return false;
  }

  const auto& var_entry = sym_table.getTabEntry(ident->tab_index);
  const auto& type_entry = sym_table.getTabEntry(canonicalType(var_entry.type));
  if (type_entry.type != static_cast<int>(semantic::BuiltinType::Array) ||
      type_entry.ref <= 0) {
    return false;
  }

  const auto& atab_entry = sym_table.getAtabEntry(type_entry.ref);
  level_diff = current_level - var_entry.lev;
  base_adr = var_entry.adr;
  array_size = std::max(1, atab_entry.size);

  node.indices[0]->accept(*this);
  emit(OpCode::LIT, 0, atab_entry.low);
  emit(OpCode::OPR, 0, 3);
  return true;
}

void CodeGenerator::emitProcOrFuncBody(ast::BlockNode* block, int block_ref) {
  const auto& block_info = sym_table.getBtabEntry(block_ref);
  emit(OpCode::INT, 0, 3 + block_info.vsze);
  if (block != nullptr) {
    block->accept(*this);
  }
  emit(OpCode::RET, 0, 0);
}

void CodeGenerator::printCode() const {
  for (size_t i = 0; i < code.size(); ++i) {
    std::cout << i << " " << code[i].toString() << " " << code[i].l << " "
              << code[i].a;
    if (code[i].aux != 0) {
      std::cout << " " << code[i].aux;
    }
    std::cout << "\n";
  }
}

void CodeGenerator::visit(ast::ProgramNode& node) {
  int jmp_main = emit(OpCode::JMP, 0, 0);

  if (node.block) {
    for (auto& decl : node.block->declarations) {
      decl->accept(*this);
    }
  }

  code[jmp_main].a = static_cast<int>(code.size());
  current_level = 1;
  emit(OpCode::INT, 0, mainFrameSize());

  if (node.block && node.block->compound_stmt) {
    node.block->compound_stmt->accept(*this);
  }

  emit(OpCode::RET, 0, 0);
}

void CodeGenerator::visit(ast::BlockNode& node) {
  for (auto& decl : node.declarations) {
    decl->accept(*this);
  }
  if (node.compound_stmt) {
    node.compound_stmt->accept(*this);
  }
}

void CodeGenerator::visit(ast::CompoundStmtNode& node) {
  for (auto& stmt : node.statements) {
    stmt->accept(*this);
  }
}

void CodeGenerator::visit(ast::AssignNode& node) {
  if (auto* arr = dynamic_cast<ast::ArrayAccessNode*>(node.target.get())) {
    int level_diff = 0;
    int base_adr = 0;
    int array_size = 0;
    if (!emitArrayRelativeIndex(*arr, level_diff, base_adr, array_size)) {
      return;
    }
    node.expr->accept(*this);
    emitStore(OpCode::STX, level_diff, base_adr, arrayElementType(*arr),
              array_size);
    return;
  }

  if (auto* rec = dynamic_cast<ast::RecordAccessNode*>(node.target.get())) {
    auto* ident = dynamic_cast<ast::IdentNode*>(rec->record_expr.get());
    if (ident != nullptr && ident->tab_index != 0 && rec->tab_index != 0) {
      node.expr->accept(*this);
      const auto& base = sym_table.getTabEntry(ident->tab_index);
      const auto& field = sym_table.getTabEntry(rec->tab_index);
      int level_diff = current_level - base.lev;
      emitStore(OpCode::STO, level_diff, base.adr + field.adr, field.type);
      return;
    }
  }

  node.expr->accept(*this);

  auto* ident = dynamic_cast<ast::IdentNode*>(node.target.get());
  if (ident == nullptr || ident->tab_index == 0) {
    return;
  }
  const auto& entry = sym_table.getTabEntry(ident->tab_index);
  int level_diff = current_level - entry.lev;
  int target_adr = entry.adr;
  if (entry.obj == semantic::ObjClass::Function && entry.ref > 0) {
    const auto& block = sym_table.getBtabEntry(entry.ref);
    target_adr = 3 + block.vsze;
    if (current_level == entry.lev + 1) {
      level_diff = 0;
    }
  }
  emitStore(OpCode::STO, level_diff, target_adr, entry.type);
}

void CodeGenerator::visit(ast::IfNode& node) {
  node.condition->accept(*this);

  int jpc_idx = emit(OpCode::JPC, 0, 0);

  node.then_branch->accept(*this);

  if (node.else_branch) {
    int jmp_idx = emit(OpCode::JMP, 0, 0);
    code[jpc_idx].a = static_cast<int>(code.size());
    node.else_branch->accept(*this);
    code[jmp_idx].a = static_cast<int>(code.size());
  } else {
    code[jpc_idx].a = static_cast<int>(code.size());
  }
}

void CodeGenerator::visit(ast::WhileNode& node) {
  int start_idx = static_cast<int>(code.size());

  node.condition->accept(*this);
  int jpc_idx = emit(OpCode::JPC, 0, 0);

  node.body->accept(*this);

  emit(OpCode::JMP, 0, start_idx);
  code[jpc_idx].a = static_cast<int>(code.size());
}

void CodeGenerator::visit(ast::RepeatNode& node) {
  int loop_start = static_cast<int>(code.size());
  for (auto& stmt : node.statements) {
    stmt->accept(*this);
  }
  node.condition->accept(*this);
  emit(OpCode::JPC, 0, loop_start);
}

void CodeGenerator::visit(ast::ForNode& node) {
  auto iter = sym_table.lookup(node.iterator.lexeme);
  if (!iter) {
    return;
  }

  node.initial->accept(*this);
  int level_diff = current_level - iter->lev;
  emitStore(OpCode::STO, level_diff, iter->adr, iter->type);

  int loop_start = static_cast<int>(code.size());
  emit(OpCode::LOD, level_diff, iter->adr);
  node.final->accept(*this);
  emit(OpCode::OPR, 0, node.is_downto ? 10 : 12);

  int jpc_exit = emit(OpCode::JPC, 0, 0);

  node.body->accept(*this);

  emit(OpCode::LOD, level_diff, iter->adr);
  emit(OpCode::LIT, 0, 1);
  emit(OpCode::OPR, 0, node.is_downto ? 3 : 2);
  emitStore(OpCode::STO, level_diff, iter->adr, iter->type);

  emit(OpCode::JMP, 0, loop_start);
  code[jpc_exit].a = static_cast<int>(code.size());
}

void CodeGenerator::visit(ast::ProcCallNode& node) {
  if (node.id.lexeme == "writeln" || node.id.lexeme == "write") {
    for (auto& arg : node.args) {
      arg->accept(*this);
      emit(OpCode::OPR, 0, 13);
    }
    if (node.id.lexeme == "writeln") {
      emit(OpCode::OPR, 0, 14);
    }
    return;
  }

  if (node.tab_index == 0) {
    return;
  }

  const auto& entry = sym_table.getTabEntry(node.tab_index);
  for (auto& arg : node.args) {
    arg->accept(*this);
  }
  int level_diff = current_level - entry.lev;
  emit(OpCode::CAL, level_diff, entry.adr);
}

void CodeGenerator::visit(ast::BinOpNode& node) {
  node.left->accept(*this);
  node.right->accept(*this);

  int opr_code = 0;
  if (node.op.type == lexer::TokenType::PLUS)
    opr_code = 2;
  else if (node.op.type == lexer::TokenType::MINUS)
    opr_code = 3;
  else if (node.op.type == lexer::TokenType::TIMES)
    opr_code = 4;
  else if (node.op.type == lexer::TokenType::IDIV ||
           node.op.type == lexer::TokenType::RDIV)
    opr_code = 5;
  else if (node.op.type == lexer::TokenType::IMOD)
    opr_code = 6;
  else if (node.op.type == lexer::TokenType::EQL)
    opr_code = 7;
  else if (node.op.type == lexer::TokenType::NEQ)
    opr_code = 8;
  else if (node.op.type == lexer::TokenType::LSS)
    opr_code = 9;
  else if (node.op.type == lexer::TokenType::GEQ)
    opr_code = 10;
  else if (node.op.type == lexer::TokenType::GTR)
    opr_code = 11;
  else if (node.op.type == lexer::TokenType::LEQ)
    opr_code = 12;
  else if (node.op.type == lexer::TokenType::ANDSY)
    opr_code = 16;
  else if (node.op.type == lexer::TokenType::ORSY)
    opr_code = 17;

  emit(OpCode::OPR, 0, opr_code);
}

void CodeGenerator::visit(ast::NumberNode& node) {
  int value = 0;
  try {
    value = std::stoi(node.val.lexeme);
  } catch (...) {
    value = static_cast<int>(std::stod(node.val.lexeme));
  }
  emit(OpCode::LIT, 0, value);
}

void CodeGenerator::visit(ast::IdentNode& node) {
  const auto& entry = sym_table.getTabEntry(node.tab_index);
  if (entry.obj == semantic::ObjClass::Constant) {
    emit(OpCode::LIT, 0, entry.adr);
    return;
  }
  int level_diff = current_level - entry.lev;
  emit(OpCode::LOD, level_diff, entry.adr);
}

void CodeGenerator::visit(ast::ConstDeclNode&) {}
void CodeGenerator::visit(ast::VarDeclNode&) {}
void CodeGenerator::visit(ast::TypeDeclNode&) {}

void CodeGenerator::visit(ast::ProcDeclNode& node) {
  auto& entry = sym_table.getTabEntry(node.tab_index);
  entry.adr = static_cast<int>(code.size());

  int saved_level = current_level;
  current_level = entry.lev + 1;

  if (entry.ref > 0) {
    emitProcOrFuncBody(node.block.get(), entry.ref);
  }

  current_level = saved_level;
}

void CodeGenerator::visit(ast::FuncDeclNode& node) {
  auto& entry = sym_table.getTabEntry(node.tab_index);
  entry.adr = static_cast<int>(code.size());

  int saved_level = current_level;
  current_level = entry.lev + 1;

  if (entry.ref > 0) {
    const auto& block_info = sym_table.getBtabEntry(entry.ref);
    emit(OpCode::INT, 0, 3 + block_info.vsze);
    if (node.block != nullptr) {
      node.block->accept(*this);
    }
    emit(OpCode::LOD, 0, 3 + block_info.vsze);
    emit(OpCode::RET, 0, 0);
  }

  current_level = saved_level;
}

void CodeGenerator::visit(ast::UnaryOpNode& node) {
  node.expr->accept(*this);
  if (node.op.type == lexer::TokenType::MINUS) {
    emit(OpCode::OPR, 0, 1);
  } else if (node.op.type == lexer::TokenType::NOTSY) {
    emit(OpCode::OPR, 0, 15);
  }
}

void CodeGenerator::visit(ast::StringNode& node) {
  const std::string& lex = node.val.lexeme;
  if (lex.size() >= 3 && lex.front() == '\'' && lex.back() == '\'') {
    emit(OpCode::LIT, 0, static_cast<int>(lex[1]));
  } else if (lex.size() >= 2 && lex.front() == '"' && lex.back() == '"') {
    for (size_t i = 1; i + 1 < lex.size(); ++i) {
      emit(OpCode::LIT, 0, static_cast<int>(lex[i]));
    }
  } else {
    emit(OpCode::LIT, 0, 0);
  }
}

void CodeGenerator::visit(ast::FuncCallNode& node) {
  if (node.tab_index == 0) {
    return;
  }
  for (auto& arg : node.args) {
    arg->accept(*this);
  }
  const auto& entry = sym_table.getTabEntry(node.tab_index);
  int level_diff = current_level - entry.lev;
  emit(OpCode::CAL, level_diff, entry.adr);
}

void CodeGenerator::visit(ast::ArrayAccessNode& node) {
  int level_diff = 0;
  int base_adr = 0;
  int array_size = 0;
  if (!emitArrayRelativeIndex(node, level_diff, base_adr, array_size)) {
    return;
  }
  emit(OpCode::LDX, level_diff, base_adr, array_size);
}

void CodeGenerator::visit(ast::RecordAccessNode& node) {
  auto* ident = dynamic_cast<ast::IdentNode*>(node.record_expr.get());
  if (ident == nullptr || ident->tab_index == 0 || node.tab_index == 0) {
    return;
  }
  const auto& base = sym_table.getTabEntry(ident->tab_index);
  const auto& field = sym_table.getTabEntry(node.tab_index);
  int level_diff = current_level - base.lev;
  emit(OpCode::LOD, level_diff, base.adr + field.adr);
}

void CodeGenerator::visit(ast::SimpleTypeSpecNode&) {}
void CodeGenerator::visit(ast::SubrangeTypeSpecNode&) {}
void CodeGenerator::visit(ast::ArrayTypeSpecNode&) {}
void CodeGenerator::visit(ast::RecordTypeSpecNode&) {}
void CodeGenerator::visit(ast::EnumTypeSpecNode&) {}

}  // namespace ir
