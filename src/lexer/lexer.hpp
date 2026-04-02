#pragma once

#include <vector>

#include "char_machine.hpp"
#include "token.hpp"

class Lexer {
 public:
  Lexer(CharMachine& reader);

  /**
   * @brief Read characters from the input and emit tokens until the end of
   * input is reached.
   *
   *
   */
  void read();

  auto tokens() const { return tokens_; }

  class State;  ///< Forward declaration of class State

 private:
  CharMachine& reader_;        ///< Character reader
  std::vector<Token> tokens_;  ///< List of tokens emitted by the lexer
  State* current_;             ///< Current state of the lexer
};