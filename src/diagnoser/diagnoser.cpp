#include "diagnoser/diagnoser.hpp"

#include <cstddef>
#include <ostream>
#include <string_view>

#include "diagnoser/color.hpp"
#include "diagnoser/diagnostic.hpp"

namespace diag {

void Diagnoser::report(Diagnostic d) { diagnostics_.emplace_back(d); }

bool Diagnoser::has_error() const {
  for (const auto& d : diagnostics_) {
    if (d.level == Level::ERROR) return true;
  }
  return false;
}

std::size_t Diagnoser::error_count() const {
  std::size_t count = 0;
  for (const auto& d : diagnostics_) {
    if (d.level == Level::ERROR) count++;
  }
  return count;
}

std::ostream& operator<<(std::ostream& os, const Diagnoser& d) {
  for (const auto& diag : d.diagnostics_) {
    d.print(os, diag);
  }
  return os;
}

std::ostream& Diagnoser::print(std::ostream& os, const Diagnostic& d) const {
  os << color::apply(source_name_ + ":" + std::to_string(d.source.line) + ":" +
                         std::to_string(d.source.col) + ": ",
                     color::BOLD);
  switch (d.phase) {
    case Phase::LEXER:
      os << "lexer ";
      break;
    case Phase::PARSER:
      os << "parser ";
      break;
    case Phase::SEMANTIC:
      os << "semantic ";
      break;
  }

  std::string_view color;

  switch (d.level) {
    case Level::ERROR:
      color = color::RED;
      os << color::apply("error: ", color);
      break;
    case Level::WARNING:
      color = color::YELLOW;
      os << color::apply("warning: ", color);
      break;
    case Level::INFO:
      color = color::CYAN;
      os << color::apply("info: ", color);
      break;
  }

  os << d.message << "\n";

  if (d.source.line > 0 && d.source.line <= lines_.size()) {
    const auto& line = lines_.at(d.source.line - 1);
    os << "\t|\n"
       << d.source.line << "\t| " << line.substr(0, d.source.col)
       << color::apply(
              line.substr(d.source.col, d.source.col + d.source.length),
              color::BOLD)
       << line.substr(d.source.col + d.source.length) << "\n";

    os << "\t|" << std::string(d.source.col, ' ')
       << color::apply(std::string(d.source.length, '^'), color) << "\n";
  }

  if (!d.hint.empty()) {
    os << "hint: " << d.hint << "\n";
  }

  return os;
}

}  // namespace diag