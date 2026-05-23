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
  std::optional<TabEntry> lookup(const std::string &id) const;

  /**
   * @brief Lookup an identifier in the tab table from the current context only
   *
   * @param id idenfiried to be looked up
   * @return std::optional<TabEntry> the result if exist, nullopt if not found
   */
  std::optional<TabEntry> lookupCurrentScope(const std::string &id) const;

  /**
   * @brief Print the tab table to the given output stream
   *
   * @param out the output stream to print to
   */
  void printTab(std::ostream &out) const;

  /**
   * @brief Print the array table to the given output stream
   *
   * @param out the output stream to print to
   */
  void printAtab(std::ostream &out) const;

  /**
   * @brief Print the block table to the given output stream
   *
   * @param out the output stream to print to
   */
  void printBtab(std::ostream &out) const;

  /**
   * @brief Get the Tab Entry object (const ref)
   *
   * @param idx global index of the entry
   * @return const TabEntry& the entry object
   */
  const TabEntry &getTabEntry(int idx) const { return tab[idx - RESERVED]; }

  /**
   * @brief Get the Tab Entry object (non const ref)
   *
   * @param idx global index of the entry
   * @return TabEntry& the entry object
   */
  TabEntry &getTabEntry(int idx) { return tab[idx - RESERVED]; }

  /**
   * @brief Get the Btab Entry object
   *
   * @param idx
   * @return const BtabEntry&
   */
  const BtabEntry &getBtabEntry(int idx) const { return btab[idx]; }

  /**
   * @brief Get the Atab Entry object
   *
   * @param idx
   * @return const AtabEntry&
   */
  const AtabEntry &getAtabEntry(int idx) const { return atab[idx]; }

 private:
  static std::string objClassName(ObjClass obj);

  std::vector<TabEntry> tab;
  std::vector<AtabEntry> atab;
  std::vector<BtabEntry> btab;
  std::vector<std::string> reserved_words;

  std::vector<int> block_stack;

  int current_level = 0;
  int last_idx = 0;
};

}  // namespace semantic
