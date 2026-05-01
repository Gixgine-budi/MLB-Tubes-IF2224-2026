#pragma once

#include <vector>

#include "arion_exceptions.hpp"
#include "char_machine.hpp"
#include "state.hpp"
#include "token.hpp"

class Lexer {
 public:
  /**
   * @brief Construct a new lexical analyzer with a valid reader
   *
   * @param reader the reference to a valids character machine reader
   */
  Lexer(CharMachine& reader);

  /**
   * @brief Read characters from the input and emit tokens until the end of
   * input is reached. Wrapper and public API for the lexer. Only need to be
   * called once.
   *
   * @throw vector<InvalidTokenException> at the end after all characters read
   */
  void process();

  /**
   * @brief Returns the list of tokens emitted by the lexer.
   *
   * @return reference to the list of tokens
   */
  const std::vector<Token>& tokens() const { return tokens_; }

 private:
  CharMachine& reader_;        ///< Character reader
  std::vector<Token> tokens_;  ///< List of raw tokens emitted by the lexer

  ///< List of invalid tokens to be thrown
  std::vector<InvalidTokenException> errors_;

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
   * @brief Handle edge case for states when reached EOF. Basically emit the
   * last token that remains in buffer and handle states with open/closing (e.g.
   * quote/string, comments)
   *
   * @return almost always returns true unless the current is a start state
   */
  bool handle_eof();

  /**
   * @brief Handle edge case to emit two tokens after accepting period after an
   * intcon (pre realcon) but the next character isn't a digit.
   *
   * @return true (emit two tokens)
   */
  bool handle_in_period_intcon();

  /**
   * @brief Check if the reader has reached eof and current sate is
   * at START state as the stop condition for the lexer.
   *
   * @return true if all characters has consumed, false otherwise
   */
  bool is_done() const;

  /**
   * @brief Consume a character from the reader and add it to the current token
   * buffer.
   *
   * @param c the character to be consumed
   */
  void consume(char c);

  /**
   * @brief Reset the state machine after emitting the token (empty buffer and
   * switch to start state)
   *
   */
  void reset();

  /**
   * @brief Parse symbol starting with character c and return the resulting
   * token type or next state.
   *
   * @param c             the character to be parsed
   * @param invalid       reference to the type of invalid state to be modified
   * if the parse result is invalid, otherwise left unchanged
   * @return StateOrToken token to be emiited or the next state
   */
  StateOrToken parse_symbol(char c, InvalidType& invalid);

  /**
   * @brief Parse a keyword from the current state of buufer and return the
   * corresponding token type.
   *
   * @return TokenType         token type (keyword) or IDENT if not a keyword
   */
  TokenType parse_keyword();
};
