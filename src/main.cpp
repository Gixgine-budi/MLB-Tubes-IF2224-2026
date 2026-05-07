#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "diagnoser/diagnoser.hpp"
#include "io/char_machine.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filepath>"
              << "\n";
    return 1;
  }

  const std::string source_name = argv[1];
  const std::string output_path = source_name + ".token";

  try {
    std::ifstream stream(source_name);
    if (!stream.is_open()) {
      throw std::runtime_error("arionin: error: cannot open '" + source_name +
                               "': " + std::strerror(errno));
    }

    io::CharMachine reader(stream, source_name);
    diag::Diagnoser diagnoser(source_name, reader.lines());
    lexer::Lexer lexer(reader, diagnoser);

    lexer.process();

    if (diagnoser.has_error()) {
      std::cerr << diagnoser;
      return 1;
    }

    std::ofstream output_file(output_path);
    if (!output_file.is_open()) {
      throw std::runtime_error("Failed to open output file: " + output_path);
    }

    auto tokens = lexer.tokens();
    for (const auto& token : tokens) {
      output_file << token << '\n';
    }

    parser::Parser parser(source_name, tokens, diagnoser);
    parser.parse();

    if (diagnoser.has_error()) {
      std::cerr << diagnoser;
      return 1;
    }

    parser.program().print();

    return 0;

  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
