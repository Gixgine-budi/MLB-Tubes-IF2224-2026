#include "semantic/symbol_table.hpp"

#include <optional>
#include <string>

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

}  // namespace

SymbolTable::SymbolTable() {
  btab.push_back(BtabEntry{0, 0, 0, 0, 0});
  block_stack.push_back(0);
  current_level = 0;
  last_idx = 0;

  auto reserve_type = [this](const std::string& name, BuiltinType type_code) {
    const int code = static_cast<int>(type_code);
    TabEntry entry{};
    entry.idx = static_cast<int>(tab.size()) + kTabReservedCount;
    entry.id = name;
    entry.link = last_idx;
    entry.obj = ObjClass::Type;
    entry.type = code;
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
    entry.idx = static_cast<int>(tab.size()) + kTabReservedCount;
    entry.id = name;
    entry.link = last_idx;
    entry.obj = ObjClass::Constant;
    entry.type = static_cast<int>(type_code);
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
}

int SymbolTable::enterTab(const std::string& id, ObjClass obj, int type,
                          int ref, int nrm, int size) {
  TabEntry new_entry{};
  new_entry.idx = static_cast<int>(tab.size()) + kTabReservedCount;
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
  copy.idx = static_cast<int>(tab.size()) + kTabReservedCount;
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

std::optional<TabEntry> SymbolTable::lookup(const std::string& id) {
  const std::string key = lower(id);
  int current_link = last_idx;
  while (current_link >= kTabReservedCount) {
    const auto& entry = getTabEntry(current_link);
    if (lower(entry.id) == key) {
      return entry;
    }
    current_link = entry.link;
  }
  return std::nullopt;
}

}  // namespace semantic
