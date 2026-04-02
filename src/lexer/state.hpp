#pragma once
#include <variant>

#include "token.hpp"

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

using StateOrToken = std::variant<State, TokenType>;
