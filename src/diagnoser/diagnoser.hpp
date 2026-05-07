#pragma once

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include "diagnoser/diagnostic.hpp"

namespace diag {

class Diagnoser {
 public:
  Diagnoser(const std::string& source_name,
            const std::vector<std::string>& lines)
      : source_name_(source_name), lines_(lines) {}

  void report(Diagnostic d);

  bool has_error() const;

  std::size_t error_count() const;

  friend std::ostream& operator<<(std::ostream& os, const Diagnoser& d);

 private:
  const std::string source_name_;
  const std::vector<std::string>& lines_;
  std::vector<Diagnostic> diagnostics_;

  std::ostream& print(std::ostream& os, const Diagnostic& d) const;
};

}  // namespace diag