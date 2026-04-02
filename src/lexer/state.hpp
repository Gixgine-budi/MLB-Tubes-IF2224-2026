#pragma once
#include <array>
#include <functional>
#include <variant>

#include "lexer.hpp"
#include "token.hpp"

enum class StateName : int {
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

using StateOrToken = std::variant<StateName, TokenType>;

class Lexer::State {
 public:
  State(StateName name, Lexer& lexer);

  bool transition();

  bool consumed() const { return buffer_.consumed; };

 private:
  StateName name_ = StateName::START;
  Token buffer_;
  Lexer& lexer_;

  std::array<std::function<StateOrToken(char)>, 20> lookup_;

  StateOrToken parse_symbol(char c);
  StateOrToken parse_keyword();
};