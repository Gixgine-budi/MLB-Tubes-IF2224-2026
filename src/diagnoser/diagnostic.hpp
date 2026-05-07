#pragma once

#include <string>

namespace diag {

enum class Phase { LEXER, PARSER, SEMANTIC };

enum class Level { ERROR, WARNING, INFO };

struct Source {
  int line = 0;  //< 0 means no location
  int col = 1;
  int length = 1;
};

struct Diagnostic {
  Phase phase;
  Level level;
  Source source;
  std::string message;
  std::string hint;
};

}  // namespace diag