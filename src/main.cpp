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

enum class RunMode { Lexer, Parser };

int main(int argc, char* argv[]) {
  std::string source_name;
  RunMode mode = RunMode::Parser;
  bool dump = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (!arg.empty() && arg[0] == '-') {
      if (arg == "--lexer" || arg == "-l") {
        mode = RunMode::Lexer;
      } else if (arg == "--parser" || arg == "-p") {
        mode = RunMode::Parser;
      } else if (arg == "--dump" || arg == "-d") {
        dump = true;
      } else {
        std::cerr << "arionin: error: unknown flag '" << arg << "'\n";
        return 1;
      }
    } else {
      if (!source_name.empty()) {
        std::cerr << "arionin: error: multiple source files specified\n";
        return 1;
      }
      source_name = arg;
    }
  }

  if (source_name.empty()) {
    // Rencana interaktif, nanti dipikir lah
    std::cerr << "arionin: interactive mode is not yet implemented\n";
    return 1;
  }

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

    auto tokens = lexer.tokens();

    if (mode == RunMode::Lexer) {
      if (dump) {
        for (const auto& token : tokens) {
          std::cout << token << '\n';
        }
      } else {
        const std::string token_path = source_name + ".token";
        std::ofstream token_file(token_path);
        if (!token_file.is_open()) {
          throw std::runtime_error("arionin: error: cannot open output file '" +
                                   token_path + "'");
        }
        for (const auto& token : tokens) {
          token_file << token << '\n';
        }
      }
      return 0;
    }

    parser::Parser parser(source_name, tokens, diagnoser);
    parser.parse();

    if (diagnoser.has_error()) {
      std::cerr << diagnoser;
      return 1;
    }

    if (dump) {
      parser.program().print();
    } else {
      const std::string ptree_path = source_name + ".ptree";
      std::ofstream ptree_file(ptree_path);
      if (!ptree_file.is_open()) {
        throw std::runtime_error("arionin: error: cannot open output file '" +
                                 ptree_path + "'");
      }
      auto* saved_buf = std::cout.rdbuf(ptree_file.rdbuf());
      parser.program().print();
      std::cout.rdbuf(saved_buf);
    }

    return 0;

  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
