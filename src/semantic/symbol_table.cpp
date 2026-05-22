#include "semantic/symbol_table.hpp"

#include <optional>
#include <string>

#include "semantic/symtable_entries.hpp"

namespace semantic {

SymbolTable::SymbolTable() {
  // TODO: Buat inisialisasi:
  // Siapkan entri untuk tipe dasar (Integer, Real, Boolean, Char, String) dan
  // kata kunci
}

int SymbolTable::pushBlock() {
  BtabEntry new_block{};
  new_block.idx = btab.size();
  new_block.last = last_idx;
  new_block.lpar = 0;
  new_block.psze = 0;
  new_block.vsze = 0;

  btab.push_back(new_block);
  current_level++;
  block_stack.push_back(new_block.idx);
  return new_block.idx;
}

void SymbolTable::popBlock() {
  if (!block_stack.empty()) {
    current_level--;
    block_stack.pop_back();
  }
}

int SymbolTable::enterTab(const std::string& id, ObjClass obj, int type,
                          int ref, int nrm, int size) {
  TabEntry new_entry{};
  new_entry.idx = tab.size();  // Offset by 33 per instructions
  new_entry.id = id;
  new_entry.link = last_idx;
  new_entry.obj = obj;
  new_entry.type = type;
  new_entry.ref = ref;
  new_entry.nrm = nrm;
  new_entry.lev = current_level;
  new_entry.adr =
      0;  // TODO: calculate based on vsze/psze depending on ObjClass

  // Update block sizes
  if (!block_stack.empty()) {
    int current_block_idx = block_stack.back();
    if (obj == ObjClass::Variable) {
      new_entry.adr = btab[current_block_idx].vsze;
      btab[current_block_idx].vsze += size;
    } else if (obj == ObjClass::Constant /* TODO: helper atau buat switch */) {
      // Determine parameter address handling based on nrm or specific Param
      // object classes btab[current_block_idx].psze += ...
    }
    btab[current_block_idx].last = new_entry.idx;
  }

  last_idx = new_entry.idx;
  tab.push_back(new_entry);
  return new_entry.idx;
}

std::optional<TabEntry> SymbolTable::lookup(const std::string& id) {
  int current_link = last_idx;
  while (current_link > 0) {
    const auto& entry = getTabEntry(current_link);
    if (entry.id == id) {
      return entry;
    }
    current_link = entry.link;
  }
  return std::nullopt;
}

}  // namespace semantic