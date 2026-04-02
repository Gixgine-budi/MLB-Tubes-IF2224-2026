#pragma once

#include <vector>

#include "char_machine.hpp"
#include "token.hpp"

class Lexer {
 public:
  class State;

  Lexer(CharMachine& reader);

  void run();

  /**
   * @brief Basic behavior: read the current character. Check the next state
   * based on the current character and current state.
   *
   * 1. If it's another state, switch to that state, consume this character to
   * buffer, then advance
   *
   * 2. If it's a token type (a final state in which there are no other way to
   * go from it), consume and return the buffer, then advance.
   *
   * 3. If there's no valid state to go or token type to return, consume and
   * return the buffer as invalid, then advance.
   *
   * @return true   if still able to advance
   * @return false  if the last character read is EOF ('\0'),
   */
  bool read();

  auto tokens() const { return tokens_; }

 private:
  CharMachine& reader_;
  std::vector<Token> tokens_;

  State* current_;
};