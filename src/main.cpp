#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "arion_exceptions.hpp"
#include "lexer/char_machine.hpp"
#include "lexer/lexer.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filepath>"
              << "\n";
    return 1;
  }

  const std::string filepath = argv[1];
  const std::string output_path = filepath + ".token";

  try {
    CharMachine reader(filepath);
    Lexer lexer(reader);
    bool lexer_error = false;

    try {
      lexer.process();
    } catch (const std::vector<InvalidTokenException>& err) {
      lexer_error = true;
      for (const auto& e : err) {
        std::cerr << e.what() << "\n";
      }
    }

    std::ofstream output_file(output_path);
    if (!output_file.is_open()) {
      throw std::runtime_error("Failed to open output file: " + output_path);
    }

    auto tokens = lexer.tokens();
    for (const auto& token : tokens) {
      output_file << token << '\n';
    }

    return lexer_error ? 1 : 0;

  } catch (const ArionException& e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
