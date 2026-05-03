#pragma once
#include <variant>

#include "token.hpp"

namespace lexer {

/**
 * @brief Enum representing the states of the lexer based on the handwritten DFA
 * in the report. The ident and keyword states is collapsed into the IN_IDENT
 *
 */
enum class State : int {
  START = 0,

  IN_QUOTE,
  IN_QUOTE_CHARCON,
  IN_CHARCON,
  END_CHARCON,
  IN_STRING,
  END_STRING,

  IN_CURLY,
  IN_PARENT,
  IN_COMMENT_PARENT,
  END_COMMENT_PARENT,

  IN_MINUS,
  IN_INTCON,
  IN_PERIOD_INTCON,
  IN_REALCON,

  IN_LESS,
  IN_EQUAL,
  IN_GREATER,
  IN_COLON,

  IN_IDENT
};

// Hold either a State or a TokenType, used for state transitions in the lexer
using StateOrToken = std::variant<State, TokenType>;

}  // namespace lexer