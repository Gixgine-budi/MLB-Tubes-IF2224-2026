#include "lexer.hpp"

#include "state.hpp"

Lexer::Lexer(CharMachine& reader) : reader_(reader) {
  current_ = new State(StateName::START, *this);
}

void Lexer::read() {
  while (current_->is_done() == false) {
    bool token_emitted = current_->transition();
  }
}