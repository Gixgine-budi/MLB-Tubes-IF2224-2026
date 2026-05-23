#pragma once

#include <string>

namespace semantic {

/**
 * @brief Constants for object classes (obj)
 *
 */
enum class ObjClass {
  Constant,
  Variable,
  Type,
  Procedure,
  Function,
  Array,
  Record
};

enum class Keyword : int {
  Program = 1,
  Procedure,
  Function,
  Begin,
  End,
  If,
  Then,
  Else,
  Case,
  While,
  Do,
  Repeat,
  Until,
  For,
  To,
  Downto,
  Var,
  Const,
  Type,
  Array,
  Record,
  Of,
  Boolean,
  Integer,
  Real,
  Char,
  String,
  Not,
  And,
  Or,
  Div,
  Mod,
};

/**
 * @brief Represents an identifier entry (tab)
 *
 */
struct TabEntry {
  int idx;        // Name index, starts from 33 (after reserved keywords)
  std::string id; // The identifier name
  int link; // Pointer/index to the previous param/variable in the same scope
  ObjClass obj; // Object class (constant, var, function, etc.)
  int type;     // Raw type of the identifier (bool, char, array, etc.)
  int ref;      // Pointer/index to other table (atab/btab) if it's an
                // array/record/procedure block
  int nrm;      // 1 normal, 0 parameter by ref
  int lev;      // Lexical level: 0 global, increments for each block
  int adr;      // Offset/address size in the stack frame
};

/**
 * @brief Represents an array entry (atab)
 *
 */
struct AtabEntry {
  int idx;  // Array entry index
  int xtyp; // Type of array index from tab
  int etyp; // Type of array element from tab
  int eref; // Pointer/index to other table if the element is composite
  int low;  // Array lower bound index
  int high; // Array upper bound index
  int elsz; // Size of an element
  int size; // Total size of the array
};

/**
 * @brief Represents a block/record entry (btab)
 *
 */
struct BtabEntry {
  int idx;  // Block entry index
  int last; // Pointer/index to the last ident declared in this block
  int lpar; // Pointer/index to the last parameter. 0 if this block is a record
  int psze; // Total parameter block size
  int vsze; // Total variable block size
};

namespace BuiltinType {
constexpr int Program = 0;
constexpr int Integer = 1;
constexpr int Real = 2;
constexpr int Boolean = 3;
constexpr int Char = 4;
constexpr int String = 5;
constexpr int Subrange = 6;
constexpr int Array = 7;
constexpr int Record = 8;
constexpr int Enumerated = 9;
constexpr int Void = 10;
}  // namespace BuiltinType

constexpr int kTabReservedCount = 33;

}  // namespace semantic
