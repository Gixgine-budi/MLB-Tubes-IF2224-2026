#include "lexer.hpp"

#include <vector>

#include "lexer/token.hpp"

Lexer::Lexer(CharMachine& reader) : reader_(reader) {}

void Lexer::process() {
  while (!is_done()) {
    if (transition()) {
      if (tokens_.back().type == TokenType::INVALID) {
        errors_.emplace_back(reader_.filepath(), tokens_.back());
      }
    }
  }

  if (!errors_.empty()) {
    throw errors_;
  }
}
