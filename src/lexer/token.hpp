#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

namespace lexer {

/**
 * @brief Enum representing the all valid token types
 *
 */
enum class TokenType {
  INVALID,      // invalid/unknown type
  INTCON,       // integer literal, e.g. 123
  REALCON,      // real literal, e.g. 3.14
  CHARCON,      // character literal, e.g. 'a' or '''
  STRING,       // string literal, e.g. 'hello' or 'it''s me'
  NOTSY,        // not
  PLUS,         // +
  MINUS,        // -
  TIMES,        // *
  IDIV,         // div
  RDIV,         // /
  IMOD,         // mod
  ANDSY,        // and
  ORSY,         // or
  EQL,          // ==
  NEQ,          // <>
  GTR,          // >
  GEQ,          // >=
  LSS,          // <
  LEQ,          // <=
  LPARENT,      // (
  RPARENT,      // )
  LBRACK,       // [
  RBRACK,       // ]
  COMMA,        // ,
  SEMICOLON,    // ;
  PERIOD,       // .
  COLON,        // :
  BECOMES,      // :=
  CONSTSY,      // const
  TYPESY,       // type
  VARSY,        // var
  FUNCTIONSY,   // function
  PROCEDURESY,  // procedure
  ARRAYSY,      // array
  RECORDSY,     // record
  PROGRAMSY,    // program
  IDENT,        // a case insensitive identifier, e.g. x or thisIsAValidIdent
  BEGINSY,      // begin
  IFSY,         // if
  CASESY,       // case
  REPEATSY,     // repeat
  WHILESY,      // while
  FORSY,        // for
  ENDSY,        // end
  ELSESY,       // else
  UNTILSY,      // until
  OFSY,         // of
  DOSY,         // do
  TOSY,         // to
  DOWNTOSY,     // downto
  THENSY,       // then
  COMMENT       // a comment, e.g. { comment } or (* comment *)
};

/**
 * @brief Enum representing the type of invalid token, used for error reporting
 *
 */
enum class InvalidType : int {
  NotInvalid = 0,
  IllegalSymbol,
  MissingQuote,
  MissingCurly,
  MissingParent,
  MissingAsterick,
  InvalidCombination,
  UnexpectedSymbol
};

/**
 * @brief Token struct representing token with type, lexeme, and source position
 * (line and column)
 *
 */
struct Token {
  TokenType type;       ///< Type of the token
  InvalidType invalid;  ///< Type of the invalid token
  std::string lexeme;   ///< Literal
  int line_num;         ///< Line number started
  int col_num;          ///< Column number started

  Token()
      : type(TokenType::INVALID),
        invalid(InvalidType::NotInvalid),
        lexeme(""),
        line_num(0),
        col_num(0) {}

  Token(TokenType type)
      : type(type),
        invalid(InvalidType::NotInvalid),
        lexeme(""),
        line_num(0),
        col_num(0) {}

  Token(TokenType type, std::string lexeme)
      : type(type),
        invalid(InvalidType::NotInvalid),
        lexeme(std::move(lexeme)),
        line_num(0),
        col_num(0) {}

  Token(TokenType type, InvalidType invalid, std::string lexeme)
      : type(type),
        invalid(invalid),
        lexeme(std::move(lexeme)),
        line_num(0),
        col_num(0) {}

  Token(TokenType type, InvalidType invalid, std::string lexeme, int line,
        int col)
      : type(type),
        invalid(invalid),
        lexeme(std::move(lexeme)),
        line_num(line),
        col_num(col) {}

  /**
   * @brief Print token in format "type (lexeme)" or "type"
   *
   * @param os                     output stream
   * @param t                      token to print
   * @return std::ostream&
   */
  friend std::ostream& operator<<(std::ostream& os, const Token& t);

  /**
   * @brief True if the token doesn't contain any lexeme (e.g. symbol token)
   *
   * @return true for empty lexeme, false otherwise
   */
  bool is_empty() const { return lexeme.empty(); }

  /**
   * @brief Return the error message given from an invalid token
   *
   * @return const std::string of the error message
   */
  const std::pair<std::string, std::string> error_hint() const;
};

/**
 * @brief Returns a human-readable display name for a token type, used in
 * parser error messages (e.g. SEMICOLON → "';'" , IDENT → "an identifier").
 */
std::string_view toString(TokenType t);

}  // namespace lexer