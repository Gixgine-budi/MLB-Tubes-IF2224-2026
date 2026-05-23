#include "semantic/symbol_table.hpp"

#include <algorithm>
#include <iomanip>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "lexer/alphabet.hpp"
#include "semantic/symtable_entries.hpp"

namespace semantic {

SymbolTable::SymbolTable() {
  reserved_words = {"<reserved>", "program", "procedure", "function", "begin",
                    "end",        "if",      "then",      "else",     "case",
                    "while",      "do",      "repeat",    "until",    "for",
                    "to",         "downto",  "var",       "const",    "type",
                    "array",      "record",  "of",        "boolean",  "integer",
                    "real",       "char",    "string",    "not",      "and",
                    "or",         "div",     "mod"};

  btab.push_back(BtabEntry{0, 0, 0, 0, 0});
  atab.push_back(AtabEntry{0, 0, 0, 0, 0, 0, 0, 0});
  block_stack.push_back(0);
  current_level = 0;
  last_idx = 0;

  auto reserve_type = [this](const std::string& name, BuiltinType type_code) {
    const int code = static_cast<int>(type_code);
    TabEntry entry{};
    entry.idx = static_cast<int>(tab.size()) + RESERVED;
    entry.id = name;
    entry.link = last_idx;
    entry.obj = ObjClass::Type;
    entry.type = entry.idx;
    entry.ref = 0;
    entry.nrm = 1;
    entry.lev = 0;
    entry.adr = code;
    last_idx = entry.idx;
    tab.push_back(entry);
    btab[0].last = last_idx;
  };

  reserve_type("integer", BuiltinType::Integer);
  reserve_type("real", BuiltinType::Real);
  reserve_type("boolean", BuiltinType::Boolean);
  reserve_type("char", BuiltinType::Char);
  reserve_type("string", BuiltinType::String);

  auto reserve_const = [this](const std::string& name, BuiltinType type_code,
                              int value) {
    TabEntry entry{};
    entry.idx = static_cast<int>(tab.size()) + RESERVED;
    entry.id = name;
    entry.link = last_idx;
    entry.obj = ObjClass::Constant;
    entry.type = RESERVED + static_cast<int>(type_code) - 1;
    entry.ref = 0;
    entry.nrm = 1;
    entry.lev = 0;
    entry.adr = value;
    last_idx = entry.idx;
    tab.push_back(entry);
    btab[0].last = last_idx;
  };

  reserve_const("true", BuiltinType::Boolean, 1);
  reserve_const("false", BuiltinType::Boolean, 0);
}

int SymbolTable::pushBlock() {
  BtabEntry new_block{};
  new_block.idx = static_cast<int>(btab.size());
  new_block.last = last_idx;
  new_block.lpar = 0;
  new_block.psze = 0;
  new_block.vsze = 0;

  btab.push_back(new_block);
  block_stack.push_back(new_block.idx);
  current_level++;
  return new_block.idx;
}

void SymbolTable::popBlock() {
  if (block_stack.size() <= 1) {
    return;
  }

  current_level--;
  block_stack.pop_back();
  const int parent_block_idx = block_stack.back();
  last_idx = btab[parent_block_idx].last;
}

int SymbolTable::enterTab(const std::string& id, ObjClass obj, int type,
                          int ref, int nrm, int size) {
  TabEntry new_entry{};
  new_entry.idx = static_cast<int>(tab.size()) + RESERVED;
  new_entry.id = id;
  new_entry.link = last_idx;
  new_entry.obj = obj;
  new_entry.type = type;
  new_entry.ref = ref;
  new_entry.nrm = nrm;
  new_entry.lev = current_level;
  new_entry.adr = 0;

  if (!block_stack.empty()) {
    int current_block_idx = block_stack.back();
    if (obj == ObjClass::Variable) {
      new_entry.adr = btab[current_block_idx].vsze;
      btab[current_block_idx].vsze += size;
    } else if (obj == ObjClass::Constant) {
      new_entry.adr = btab[current_block_idx].psze;
      btab[current_block_idx].psze += size;
    }
    btab[current_block_idx].last = new_entry.idx;
  }

  last_idx = new_entry.idx;
  tab.push_back(new_entry);
  return new_entry.idx;
}

int SymbolTable::enterTab(const TabEntry entry) {
  TabEntry copy = entry;
  copy.idx = static_cast<int>(tab.size()) + RESERVED;
  copy.link = last_idx;
  copy.lev = current_level;
  last_idx = copy.idx;
  if (!block_stack.empty()) {
    btab[block_stack.back()].last = last_idx;
  }
  tab.push_back(copy);
  return copy.idx;
}

int SymbolTable::enterTab(const AtabEntry entry) {
  AtabEntry copy = entry;
  copy.idx = static_cast<int>(atab.size());
  atab.push_back(copy);
  return copy.idx;
}

int SymbolTable::enterTab(const BtabEntry entry) {
  BtabEntry copy = entry;
  copy.idx = static_cast<int>(btab.size());
  btab.push_back(copy);
  return copy.idx;
}

std::optional<TabEntry> SymbolTable::lookup(const std::string& id) const {
  const std::string key = lexer::Alphabet::to_lower(id);
  int current_link = last_idx;
  while (current_link >= RESERVED) {
    const auto& entry = getTabEntry(current_link);
    if (lexer::Alphabet::to_lower(entry.id) == key) {
      return entry;
    }
    current_link = entry.link;
  }
  return std::nullopt;
}

std::optional<TabEntry> SymbolTable::lookupCurrentScope(
    const std::string& id) const {
  const std::string key = lexer::Alphabet::to_lower(id);
  int current_link = last_idx;

  while (current_link >= RESERVED) {
    const auto& entry = getTabEntry(current_link);
    if (entry.lev < current_level) {
      break;
    }
    if (entry.lev == current_level &&
        lexer::Alphabet::to_lower(entry.id) == key) {
      return entry;
    }
    current_link = entry.link;
  }

  return std::nullopt;
}

std::string SymbolTable::objClassName(ObjClass obj) {
  switch (obj) {
    case ObjClass::Constant:
      return "constant";
    case ObjClass::Variable:
      return "variable";
    case ObjClass::Type:
      return "type";
    case ObjClass::Procedure:
      return "procedure";
    case ObjClass::Function:
      return "function";
    case ObjClass::Array:
      return "array";
    case ObjClass::Record:
      return "record";
  }
  return "unknown";
}

void SymbolTable::printTab(std::ostream& out) const {
  std::size_t id_width = 2;
  std::size_t obj_width = 3;

  for (const auto& name : reserved_words) {
    id_width = std::max(id_width, name.size());
  }
  for (const auto& entry : tab) {
    id_width = std::max(id_width, entry.id.size());
    obj_width = std::max(obj_width, objClassName(entry.obj).size());
  }

  const std::size_t idx_width = 3;
  const std::size_t type_width = 4;
  const std::size_t ref_width = 3;
  const std::size_t nrm_width = 3;
  const std::size_t lev_width = 3;
  const std::size_t adr_width = 3;
  const std::size_t link_width = 4;

  auto print_separator = [&]() {
    out << '|';
    out << std::string(idx_width + 2, '-') << '|';
    out << std::string(id_width + 2, '-') << '|';
    out << std::string(obj_width + 2, '-') << '|';
    out << std::string(type_width + 2, '-') << '|';
    out << std::string(ref_width + 2, '-') << '|';
    out << std::string(nrm_width + 2, '-') << '|';
    out << std::string(lev_width + 2, '-') << '|';
    out << std::string(adr_width + 2, '-') << '|';
    out << std::string(link_width + 2, '-') << "|\n";
  };

  out << "| " << std::setw(static_cast<int>(idx_width)) << std::right << "idx"
      << " | " << std::setw(static_cast<int>(id_width)) << std::left << "id"
      << " | " << std::setw(static_cast<int>(obj_width)) << std::left << "obj"
      << " | " << std::setw(static_cast<int>(type_width)) << std::right
      << "type"
      << " | " << std::setw(static_cast<int>(ref_width)) << std::right << "ref"
      << " | " << std::setw(static_cast<int>(nrm_width)) << std::right << "nrm"
      << " | " << std::setw(static_cast<int>(lev_width)) << std::right << "lev"
      << " | " << std::setw(static_cast<int>(adr_width)) << std::right << "adr"
      << " | " << std::setw(static_cast<int>(link_width)) << std::right
      << "link"
      << " |\n";
  print_separator();

  for (int i = 0; i < RESERVED; ++i) {
    const std::string& keyword = (i < static_cast<int>(reserved_words.size()))
                                     ? reserved_words[i]
                                     : std::string("<reserved>");
    out << "| " << std::setw(static_cast<int>(idx_width)) << std::right << i
        << " | " << std::setw(static_cast<int>(id_width)) << std::left
        << keyword << " | " << std::setw(static_cast<int>(obj_width))
        << std::left << "keyword"
        << " | " << std::setw(static_cast<int>(type_width)) << std::right << 0
        << " | " << std::setw(static_cast<int>(ref_width)) << std::right << 0
        << " | " << std::setw(static_cast<int>(nrm_width)) << std::right << 0
        << " | " << std::setw(static_cast<int>(lev_width)) << std::right << 0
        << " | " << std::setw(static_cast<int>(adr_width)) << std::right << 0
        << " | " << std::setw(static_cast<int>(link_width)) << std::right << 0
        << " |\n";
  }

  for (const auto& entry : tab) {
    out << "| " << std::setw(static_cast<int>(idx_width)) << std::right
        << entry.idx << " | " << std::setw(static_cast<int>(id_width))
        << std::left << entry.id << " | "
        << std::setw(static_cast<int>(obj_width)) << std::left
        << objClassName(entry.obj) << " | "
        << std::setw(static_cast<int>(type_width)) << std::right << entry.type
        << " | " << std::setw(static_cast<int>(ref_width)) << std::right
        << entry.ref << " | " << std::setw(static_cast<int>(nrm_width))
        << std::right << entry.nrm << " | "
        << std::setw(static_cast<int>(lev_width)) << std::right << entry.lev
        << " | " << std::setw(static_cast<int>(adr_width)) << std::right
        << entry.adr << " | " << std::setw(static_cast<int>(link_width))
        << std::right << entry.link << " |\n";
  }
}

void SymbolTable::printAtab(std::ostream& out) const {
  const int idx_width = 3;
  const int xtyp_width = 4;
  const int etyp_width = 4;
  const int eref_width = 4;
  const int low_width = 3;
  const int high_width = 4;
  const int elsz_width = 4;
  const int size_width = 4;

  out << "| " << std::setw(idx_width) << std::right << "idx"
      << " | " << std::setw(xtyp_width) << std::right << "xtyp"
      << " | " << std::setw(etyp_width) << std::right << "etyp"
      << " | " << std::setw(eref_width) << std::right << "eref"
      << " | " << std::setw(low_width) << std::right << "low"
      << " | " << std::setw(high_width) << std::right << "high"
      << " | " << std::setw(elsz_width) << std::right << "elsz"
      << " | " << std::setw(size_width) << std::right << "size"
      << " |\n";

  out << '|';
  out << std::string(idx_width + 2, '-') << '|';
  out << std::string(xtyp_width + 2, '-') << '|';
  out << std::string(etyp_width + 2, '-') << '|';
  out << std::string(eref_width + 2, '-') << '|';
  out << std::string(low_width + 2, '-') << '|';
  out << std::string(high_width + 2, '-') << '|';
  out << std::string(elsz_width + 2, '-') << '|';
  out << std::string(size_width + 2, '-') << "|\n";

  for (std::size_t i = 1; i < atab.size(); ++i) {
    const auto& entry = atab[i];
    out << "| " << std::setw(idx_width) << std::right << entry.idx << " | "
        << std::setw(xtyp_width) << std::right << entry.xtyp << " | "
        << std::setw(etyp_width) << std::right << entry.etyp << " | "
        << std::setw(eref_width) << std::right << entry.eref << " | "
        << std::setw(low_width) << std::right << entry.low << " | "
        << std::setw(high_width) << std::right << entry.high << " | "
        << std::setw(elsz_width) << std::right << entry.elsz << " | "
        << std::setw(size_width) << std::right << entry.size << " |\n";
  }
}

void SymbolTable::printBtab(std::ostream& out) const {
  const int idx_width = 3;
  const int last_width = 4;
  const int lpar_width = 4;
  const int psze_width = 4;
  const int vsze_width = 4;

  out << "| " << std::setw(idx_width) << std::right << "idx"
      << " | " << std::setw(last_width) << std::right << "last"
      << " | " << std::setw(lpar_width) << std::right << "lpar"
      << " | " << std::setw(psze_width) << std::right << "psze"
      << " | " << std::setw(vsze_width) << std::right << "vsze"
      << " |\n";

  out << '|';
  out << std::string(idx_width + 2, '-') << '|';
  out << std::string(last_width + 2, '-') << '|';
  out << std::string(lpar_width + 2, '-') << '|';
  out << std::string(psze_width + 2, '-') << '|';
  out << std::string(vsze_width + 2, '-') << "|\n";

  for (const auto& entry : btab) {
    out << "| " << std::setw(idx_width) << std::right << entry.idx << " | "
        << std::setw(last_width) << std::right << entry.last << " | "
        << std::setw(lpar_width) << std::right << entry.lpar << " | "
        << std::setw(psze_width) << std::right << entry.psze << " | "
        << std::setw(vsze_width) << std::right << entry.vsze << " |\n";
  }
}

}  // namespace semantic
