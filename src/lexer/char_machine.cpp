#include "char_machine.hpp"

#include <cassert>
#include <cerrno>
#include <system_error>

#include "arion_exceptions.hpp"

CharMachine::CharMachine(const std::string& filepath) : filepath_(filepath) {
  stream_.open(filepath_);
  if (!stream_.is_open()) {
    auto err = std::system_error(errno, std::generic_category());
    throw FileErrorException(filepath_, err.what());
  }
  stream_.get(current_);
}

CharMachine::~CharMachine() {
  if (stream_.is_open()) {
    stream_.close();
  }
}

bool CharMachine::advance() {
  if (!stream_.is_open())
    throw FileErrorException(filepath_, "stream is not open");
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