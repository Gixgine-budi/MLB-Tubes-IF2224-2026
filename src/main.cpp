#include <iostream>
#include <string>

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

    std::cout << "Lexical analysis completed successfully.\n";
    std::cout << "Read " << reader.line_num() << " lines. and "
              << lexer.tokens().size() << " tokens.\n";

    std::cout << "Enter output file path > ";
    std::getline(std::cin, filepath);
    std::ofstream output_file(filepath);
    if (!output_file.is_open())
      throw std::runtime_error("Failed to open output file: " + filepath);

    auto tokens = lexer.tokens();
    for (const auto& token : tokens) {
      output_file << token << '\n';
    }

    std::cout << "Tokens have been written to " << filepath << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
