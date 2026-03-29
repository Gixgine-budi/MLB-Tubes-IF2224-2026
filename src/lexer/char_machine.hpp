#pragma once

#include <fstream>
#include <string>

/**
 * Buffered single-character reader over a std::ifstream
 * After construction, the first input character is already loaded (or EOF is
 * set for an empty file). Use eof() before current() when at end of file;
 */
class CharMachine {
 private:
  std::ifstream stream_;
  char current_ = '\0';
  int line_num_ = 1;
  int col_num_ = 1;

 public:
  CharMachine(const std::string& filepath);
  ~CharMachine();

  CharMachine(const CharMachine&) = delete;
  CharMachine& operator=(const CharMachine&) = delete;

  /**
   * @brief Advance the character machine by one character.
   *
   * @return bool               True if the character was read successfully,
   * false if the end of the stream was reached.
   * @throws std::runtime_error If the stream is closed.
   */
  bool advance();

  /**
   * @brief Get the current character.
   *
   * @return char            The current character.
   */
  char current() const { return current_; }

  /**
   * @brief Get the current column number
   *
   * @return int             column number (1 based)
   */
  int col_num() const { return col_num_; }

  /**
   * @brief Get the current line number
   *
   * @return int             column number (1 based)
   */
  int line_num() const { return line_num_; }

  /**
   * @brief Check if the character machine is at the end of the stream.
   *
   * @return bool            True if the end of the stream was reached, false
   * otherwise.
   */
  bool eof() const noexcept { return stream_.eof(); }
};
