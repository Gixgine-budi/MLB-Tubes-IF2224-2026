#include "char_machine.hpp"

#include <cassert>
#include <stdexcept>

CharMachine::CharMachine(const std::string& filepath) {
  stream_.open(filepath);
  if (!stream_.is_open())
    throw std::runtime_error("Failed to open file " + filepath);
  stream_.get(current_);
}

CharMachine::~CharMachine() {
  if (stream_.is_open()) {
    stream_.close();
  }
}

bool CharMachine::advance() {
  if (!stream_.is_open()) throw std::runtime_error("Stream is not open");
  if (eof()) {
    current_ = '\0';
    return false;
  }

  stream_.get(current_);
  if (current_ == '\n') {
    line_num_++;
    col_num_ = 1;
  } else {
    col_num_++;
  }

  return true;
}