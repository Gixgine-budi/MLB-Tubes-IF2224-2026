#include "diagnoser/diagnoser.hpp"

#include <algorithm>
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
  // Format header:
  // "file:line:col: phase level: message"
  // or "file: phase level: message"

  std::string header = source_name_;
  if (d.source.line > 0) {
    header += ":" + std::to_string(d.source.line) + ":" +
              std::to_string(d.source.col);
  }
  header += ": ";
  os << color::apply(header, color::BOLD);

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

  std::string_view level_color;
  switch (d.level) {
    case Level::ERROR:
      level_color = color::RED;
      os << color::apply("error: ", level_color);
      break;
    case Level::WARNING:
      level_color = color::YELLOW;
      os << color::apply("warning: ", level_color);
      break;
    case Level::INFO:
      level_color = color::CYAN;
      os << color::apply("info: ", level_color);
      break;
  }

  os << d.message << "\n";

  if (d.source.line > 0 && d.source.line <= static_cast<int>(lines_.size())) {
    const auto& line = lines_.at(d.source.line - 1);
    const int pad_w = static_cast<int>(std::to_string(d.source.line).size());
    const std::string pad(pad_w, ' ');

    const int col0 = std::max(1, d.source.col) - 1;
    const int len = std::max(1, d.source.length);
    const int suffix = std::min(col0 + len, static_cast<int>(line.size()));

    os << color::apply("  " + std::to_string(d.source.line) + " | ", color::DIM)
       << line.substr(0, col0)
       << color::apply(line.substr(col0, suffix - col0), color::BOLD)
       << line.substr(suffix);
    os << color::apply("  " + pad + " | ", color::DIM) << std::string(col0, ' ')
       << color::apply(std::string(suffix - col0, '^'), level_color) << "\n";
  }

  if (!d.hint.empty()) {
    os << color::apply("hint: ", color::CYAN) << d.hint;
  }

  os << "\n";
  return os;
}

}  // namespace diag