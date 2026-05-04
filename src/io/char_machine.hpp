#pragma once

#include <istream>
#include <string>
#include <vector>

namespace io {

/**
 * Buffered single-character reader over a std::ifstream
 * After construction, the first input character is already loaded (or EOF is
 * set for an empty file). Use eof() before current() when at end of file;
 */
class CharMachine {
 public:
  /**
   * @brief Construct a new character machine reader
   *
   * @param stream the input stream to read from (e.g. ifstream, stdin). The
   * stream must be open and valid for reading.
   * @param source_name the name of the source (e.g. file path, stdin)
   * associated with the stream
   * @throws std::ios_base::failure if the stream is not valid for reading
   */
  CharMachine(std::istream& stream, const std::string& source_name);

  ~CharMachine() = default;
  CharMachine(const CharMachine&) = delete;
  CharMachine& operator=(const CharMachine&) = delete;

  /**
   * @brief Get the filepath associated with the reader
   *
   * @return const std::string& reference to the file path
   */
  const std::string& filepath() const { return source_name_; }

  /**
   * @brief Advance the character machine by one character.
   *
   * @return bool True if the character was read successfully,
   * false if the end of the stream was reached.
   */
  bool advance();

  /**
   * @brief Get the current character.
   *
   * @return char
   */
  char current() const { return current_char_; }

  /**
   * @brief Get the current line read so far (might be incomplete)
   *
   * @return const std::string& reference to the current line of text
   */
  const std::string& current_line() const { return current_line_; }

  /**
   * @brief Get all lines read so far excluding current line
   *
   * @return const std::vector<std::string>& reference to the vector of lines
   * read so far
   */
  const std::vector<std::string>& lines() const { return lines_; }

  /**
   * @brief Get the current column number
   *
   * @return int column number (1 based)
   */
  int col_num() const { return col_num_; }

  /**
   * @brief Get the current line number
   *
   * @return int line number (1 based)
   */
  int line_num() const { return line_num_; }

  /**
   * @brief Check if the character machine is at the end of the stream.
   *
   * @return bool True if the end of the stream was reached, false otherwise.
   */
  bool eof() const noexcept { return stream_.eof(); }

 private:
  std::istream& stream_;
  const std::string source_name_;

  char current_char_ = '\0';
  int line_num_ = 1;
  int col_num_ = 1;

  std::vector<std::string> lines_;
  std::string current_line_;
};

}  // namespace io