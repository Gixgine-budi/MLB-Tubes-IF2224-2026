#include <iostream>

#include "lexer/char_machine.hpp"
#include "lexer/lexer.hpp"

int main(int argc, char* argv[]) {
  std::string filepath;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
    return 1;
  }

  filepath = argv[1];

  // Lexical Analyzer
  try {
    CharMachine reader(filepath);
    Lexer lexer(reader);
    lexer.run();

    auto tokens = lexer.tokens();
    for (const auto& token : tokens) {
      std::cout << token << '\n';
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
