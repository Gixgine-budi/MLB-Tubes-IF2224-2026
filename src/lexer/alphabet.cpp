#include "alphabet.hpp"

#include <algorithm>
#include <cctype>
#include <string>

const std::string Alphabet::symbols = "+-*/=<>()[]{}.,;:\'";

bool Alphabet::is_letter(char c) {
  c = std::tolower(c);
  return ('a' <= c && c <= 'z');
}

bool Alphabet::is_digit(char c) { return ('0' <= c && c <= '9'); }

bool Alphabet::is_whitespace(char c) {
  return (' ' == c || '\t' == c || '\n' == c || '\r' == c);
}

bool Alphabet::is_symbol(char c) {
  return symbols.find(c) != std::string::npos;
}

std::string Alphabet::to_lower(const std::string& s) {
  std::string lower = s;
  std::for_each(lower.begin(), lower.end(),
                [](char& c) { c = (char)tolower(c); });
  return lower;
}