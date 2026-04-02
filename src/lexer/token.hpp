#pragma once

#include <ostream>
#include <string>

/**
 * @brief Enum representing the all valid token types
 *
 */
enum class TokenType {
  INVALID,
  INTCON,
  REALCON,
  CHARCON,
  STRING,
  NOTSY,
  PLUS,
  MINUS,
  TIMES,
  IDIV,
  RDIV,
  IMOD,
  ANDSY,
  ORSY,
  EQL,
  NEQ,
  GTR,
  GEQ,
  LSS,
  LEQ,
  LPARENT,
  RPARENT,
  LBRACK,
  RBRACK,
  COMMA,
  SEMICOLON,
  PERIOD,
  COLON,
  BECOMES,
  CONSTSY,
  TYPESY,
  VARSY,
  FUNCTIONSY,
  PROCEDURESY,
  ARRAYSY,
  RECORDSY,
  PROGRAMSY,
  IDENT,
  BEGINSY,
  IFSY,
  CASESY,
  REPEATSY,
  WHILESY,
  FORSY,
  ENDSY,
  ELSESY,
  UNTILSY,
  OFSY,
  DOSY,
  TOSY,
  DOWNTOSY,
  THENSY,
  COMMENT
};

/**
 * @brief Token struct representing token with type, lexeme, and source position
 * (line and column)
 *
 */
struct Token {
  TokenType type;
  std::string lexeme;
  int line_num;
  int col_num;
  bool consumed = false;

  /**
   * @brief Print token in format "type(lexeme)" or "type"
   *
   * @param os                     output stream
   * @param t                      token to print
   * @return std::ostream&
   */
  friend std::ostream& operator<<(std::ostream& os, const Token& t);

  void consume(char c) {
    lexeme.append(std::string(1, c));
    consumed = true;
  }

  bool is_empty() const { return lexeme.empty(); }
};
