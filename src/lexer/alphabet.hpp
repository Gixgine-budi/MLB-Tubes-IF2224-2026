#pragma once
#include <string>

/**
 * @brief Valid character sets and helper functions for character classification
 *
 */
struct Alphabet {
  static const std::string letters;
  static const std::string digits;
  static const std::string whitespace;
  static const std::string symbols;

  /**
   * @brief Check if a character is a letter (a-z) case-insensitive
   *
   * @param c
   * @return true
   * @return false
   */
  static bool is_letter(char c);

  /**
   * @brief Check if a character is a digit (0-9)
   *
   * @param c
   * @return true
   * @return false
   */
  static bool is_digit(char c);

  /**
   * @brief Check if a character is a whitespace character (space, tab, newline,
   * carriage return)
   *
   * @param c
   * @return true
   * @return false
   */
  static bool is_whitespace(char c);

  /**
   * @brief Check if a character is a symbol (see Alphabet::symbols)
   *
   * @param c
   * @return true
   * @return false
   */
  static bool is_symbol(char c);

  /**
   * @brief Lower case the whole string
   *
   * @param s constnt reference to the string
   * @return std::string a new string where all ascii alphabet are lowercase
   */
  static std::string to_lower(const std::string& s);
};