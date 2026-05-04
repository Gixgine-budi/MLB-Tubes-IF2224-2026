#include "char_machine.hpp"

#include <ios>
#include <istream>
#include <string>

namespace io {

CharMachine::CharMachine(std::istream& stream, const std::string& source_name)
    : stream_(stream), source_name_(source_name) {
  if (!stream_) throw std::ios_base::failure("invalid input stream");
  if (stream_.get(current_char_)) current_line_ += current_char_;
}

bool CharMachine::advance() {
  if (eof() || !stream_) return false;

  if (current_char_ == '\n') {
    lines_.push_back(current_line_);
    current_line_.clear();
    line_num_++;
    col_num_ = 1;
  } else {
    col_num_++;
  }

  if (stream_.get(current_char_)) {
    current_line_ += current_char_;
    return true;
  }

  if (!current_line_.empty()) {
    lines_.push_back(current_line_);
  }

  current_char_ = '\0';
  return false;
}

}  // namespace io