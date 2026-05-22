#pragma once

#include <optional>
#include <string>
#include <vector>

#include "symtable_entries.hpp"

namespace semantic {

// Note: jujur ini 33 nya masih di hardcode tolong nanti implementing yak

class SymbolTable {
public:
  SymbolTable();

  /**
   * @brief Pushes a new block onto the stack, tracking variables and lexical
   * level
   *
   * @return int the new block index
   */
  int pushBlock();

  // Pops the current block, returning to the previous lexical scope
  /**
   * @brief Pops the current block, returning to the previous lexical scope
   *
   */
  void popBlock();

  //
  /**
   * @brief Enter a new identifier into the tab table
   *
   * @param id
   * @param obj
   * @param type
   * @param ref
   * @param nrm
   * @param size
   * @return int
   */
  int enterTab(const std::string &id, ObjClass obj, int type, int ref = 0,
               int nrm = 1, int size = 1);

  /**
   * @brief TODO: deskripsi dan implemment
   *
   * @param entry
   * @return int
   */
  int enterTab(const TabEntry entry);

  /**
   * @brief TODO: deskripsi dan implemment
   *
   * @param entry
   * @return int
   */
  int enterTab(const AtabEntry entry);

  /**
   * @brief TODO: deskripsi dan implemment
   *
   * @param entry
   * @return int
   */
  int enterTab(const BtabEntry entry);

  /**
   * @brief Lookup an identifier in the tab table from the current context
   * upwards
   *
   * @param id idenfiried to be looked up
   * @return std::optional<TabEntry> the result if exist, nullopt if not found
   */
  std::optional<TabEntry> lookup(const std::string &id);

  const TabEntry &getTabEntry(int idx) const { return tab[idx - 33]; }

  TabEntry &getTabEntry(int idx) { return tab[idx - 33]; }

  const BtabEntry &getBtabEntry(int idx) const { return btab[idx]; }

  const AtabEntry &getAtabEntry(int idx) const { return atab[idx]; }

private:
  std::vector<TabEntry> tab;
  std::vector<AtabEntry> atab;
  std::vector<BtabEntry> btab;

  std::vector<int> block_stack;

  int current_level = 0;
  int last_idx = 0;
};

} // namespace semantic
