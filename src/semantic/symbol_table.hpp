#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "symtable_entries.hpp"

namespace semantic {

class SymbolTable {
 public:
  SymbolTable();

  int pushBlock();
  void popBlock();

  int enterTab(const std::string &id, ObjClass obj, int type, int ref = 0,
               int nrm = 1, int size = 1);

  int enterParam(const std::string &id, int type, int nrm = 1, int size = 1);

  int enterTab(const TabEntry entry);
  int enterTab(const AtabEntry entry);
  int enterTab(const BtabEntry entry);

  std::optional<TabEntry> lookup(const std::string &id) const;
  std::optional<TabEntry> lookupCurrentScope(const std::string &id) const;

  void printTab(std::ostream &out) const;
  void printAtab(std::ostream &out) const;
  void printBtab(std::ostream &out) const;

  const TabEntry &getTabEntry(int idx) const { return tab[idx - RESERVED]; }
  TabEntry &getTabEntry(int idx) { return tab[idx - RESERVED]; }
  const BtabEntry &getBtabEntry(int idx) const { return btab[idx]; }
  const AtabEntry &getAtabEntry(int idx) const { return atab[idx]; }

  int currentLevel() const { return current_level; }
  int currentBlockIdx() const { return block_stack.back(); }

 private:
  static std::string objClassName(ObjClass obj);
  int appendTabEntry(TabEntry entry, int block_idx, bool is_param);

  std::vector<TabEntry> tab;
  std::vector<AtabEntry> atab;
  std::vector<BtabEntry> btab;
  std::vector<std::string> reserved_words;

  std::vector<int> block_stack;
  std::vector<int> display;

  int current_level = 0;
};

}  // namespace semantic
