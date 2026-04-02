#include "lexer.hpp"

Lexer::Lexer(CharMachine& reader) : reader_(reader) {}

void Lexer::read() {
  while (!is_done()) {
    bool token_emitted = transition();
  }
}
