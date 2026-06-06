#include "lexer/lexer.hpp"

#include <ostream>

#include "diagnoser/diagnoser.hpp"
#include "io/char_machine.hpp"

namespace lexer {

Lexer::Lexer(io::CharMachine& reader, diag::Diagnoser& diagnoser)
    : reader_(reader), diagnoser_(diagnoser) {}

void Lexer::process() {
  while (!is_done()) {
    transition();
  }
}

void Lexer::print(std::ostream& os) const {
  for (auto& token : tokens_) {
    os << token << '\n';
  }
}

}  // namespace lexer