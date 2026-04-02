#include "lexer.hpp"

#include <iostream>

#include "state.hpp"

Lexer::Lexer(CharMachine& reader) : reader_(reader) {
  current_ = new State(StateName::START, *this);
}

bool Lexer::read() {
  if (current_->transition()) {
    if (tokens_.back().type == TokenType::INVALID) {
      std::cerr << "Invalid token at line " << tokens_.back().line_num
                << ", column " << tokens_.back().col_num << ": "
                << tokens_.back().lexeme << std::endl;
    }
  }

  return reader_.advance();
}

void Lexer::run() { while (read()); }