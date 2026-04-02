#pragma once

#include <vector>

#include "char_machine.hpp"
#include "state.hpp"
#include "token.hpp"

class Lexer {
 public:
  Lexer(CharMachine& reader);

  /**
   * @brief Read characters from the input and emit tokens until the end of
   * input is reached.
   */
  void read();

  auto tokens() const { return tokens_; }

 private:
  CharMachine& reader_;        ///< Character reader
  std::vector<Token> tokens_;  ///< List of tokens emitted by the lexer
  
  State current_ = State::START;
  Token buffer_;

  bool transition();
  bool is_done() const;
  void consume(char c);
  StateOrToken parse_symbol(char c);
  StateOrToken parse_keyword();
};
