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
   * input is reached. Wrapper and public API for the lexer.
   */
  void read();

  /**
   * @brief Returns the list of tokens emitted by the lexer.
   *
   * @return auto
   */
  auto tokens() const { return tokens_; }

 private:
  CharMachine& reader_;        ///< Character reader
  std::vector<Token> tokens_;  ///< List of tokens emitted by the lexer

  State current_ = State::START;  ///< Current state of the lexer
  Token buffer_;  ///< Buffer for the current token being processed

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
   * 3. In some cases, the next state can be a token type or another state
   * depending on the current character. In this case, return the current buffer
   * as a token if it's not empty, then restart the state machine with the
   * current character (without consuming it).
   *
   * 4. If there's no valid state to go or token type to return, consume and
   * return the buffer as invalid, then advance.
   *
   * @return true   if a token is emitted
   * @return false  otherwise
   */
  bool transition();

  /**
   * @brief Check if the reader has reached the end of input and current sate is
   * at START state as the stop condition for the lexer.
   *
   * @return true
   * @return false
   */
  bool is_done() const;

  /**
   * @brief Consume a character from the reader and add it to the current token
   * buffer.
   *
   * @param c
   */
  void consume(char c);

  /**
   * @brief Parse symbol starting with character c and return the resulting
   * token type or next state.
   *
   * @param c                     current character
   * @return StateOrToken         next state or token type
   */
  StateOrToken parse_symbol(char c);

  /**
   * @brief Parse a keyword from the input and return the corresponding token
   * type.
   *
   * @return StateOrToken         token type (keyword) or IDENT if not a keyword
   */
  StateOrToken parse_keyword();
};
