#pragma once

#ifdef _WIN32
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

#include <string>
#include <string_view>

namespace diag::color {

/**
 * @brief Check if the output is a terminal to determine if color codes should
 * be applied.
 *
 * @return true if the output is a terminal, false otherwise
 */
inline bool enabled() {
  static bool is_terminal = ISATTY(FILENO(stderr));
  return is_terminal;
}

constexpr std::string_view RESET = "\033[0m";
constexpr std::string_view BOLD = "\033[1m";
constexpr std::string_view DIM = "\033[2m";
constexpr std::string_view RED = "\033[31m";
constexpr std::string_view YELLOW = "\033[33m";
constexpr std::string_view CYAN = "\033[36m";

/**
 * @brief Apply the given color code to the text if color is enabled return the
 * original text otherwise.
 *
 * @param text The text to which the color code should be applied.
 * @param code The color code to apply.
 * @return std::string The text with the color code applied if enabled,
 * otherwise the original text.
 */
inline std::string apply(std::string_view text, std::string_view code) {
  if (enabled()) {
    return std::string(code) + std::string(text) + std::string(RESET);
  }
  return std::string(text);
}
}  // namespace diag::color