#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "diagnoser/color.hpp"
#include "diagnoser/diagnoser.hpp"
#include "io/char_machine.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/ast_printer.hpp"
#include "semantic/semantic_analyzer.hpp"
#include "icg/code_generator.hpp"
#include "interpreter/interpreter.hpp"

enum class RunMode { Lexer, Parser, Semantic };

namespace {

bool hasUTF8(const char* value) {
  if (value == nullptr) return false;

  std::string normalized(value);
  for (char& ch : normalized) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  }

  return normalized.find("UTF-8") != std::string::npos ||
         normalized.find("UTF8") != std::string::npos;
}

bool useASCII() {
  if (!diag::color::enabled()) return true;

  const char* lc_all = std::getenv("LC_ALL");
  if (lc_all != nullptr && lc_all[0] != '\0') return hasUTF8(lc_all);

  const char* lc_ctype = std::getenv("LC_CTYPE");
  if (lc_ctype != nullptr && lc_ctype[0] != '\0') return hasUTF8(lc_ctype);

  const char* lang = std::getenv("LANG");
  if (lang != nullptr && lang[0] != '\0') return hasUTF8(lang);

  return false;
}

std::unique_ptr<std::ofstream> openOutputFile(const std::string& path) {
  auto file = std::make_unique<std::ofstream>(path);
  if (!file->is_open()) {
    throw std::runtime_error("arion: error: cannot open output file '" + path +
                             "'");
  }
  return file;
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string source_name;
  RunMode mode = RunMode::Semantic;
  bool dump = false;
  bool dump_all = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (!arg.empty() && arg[0] == '-') {
      if (arg == "--lexer" || arg == "-l") {
        mode = RunMode::Lexer;
      } else if (arg == "--parser" || arg == "-p") {
        mode = RunMode::Parser;
      } else if (arg == "--semantic" || arg == "-s") {
        mode = RunMode::Semantic;
      } else if (arg == "--dump" || arg == "-d") {
        dump = true;
      } else if (arg == "--dump-all") {
        dump_all = true;
      } else {
        std::cerr << "arion: error: unknown flag '" << arg << "'\n";
        return 1;
      }
    } else {
      if (!source_name.empty()) {
        std::cerr << "arion: error: multiple source files specified\n";
        return 1;
      }
      source_name = arg;
    }
  }

  if (source_name.empty()) {
    std::cerr << "arion: error: no source file specified\n";
    return 1;
  }

  if (dump && dump_all) {
    std::cerr
        << "arion: error: --dump and --dump-all cannot be used together\n";
    return 1;
  }

  try {
    std::ifstream stream(source_name);
    if (!stream.is_open()) {
      throw std::runtime_error("arion: error: cannot open '" + source_name +
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

    const bool ascii = useASCII();

    auto writeTokenFile = [&]() {
      const std::string token_path = source_name + ".token";
      auto token_file = openOutputFile(token_path);
      lexer.print(*token_file);
    };

    auto dumpTokens = [&]() { lexer.print(std::cout); };

    if (mode == RunMode::Lexer) {
      if (dump || dump_all) {
        if (dump_all) {
          std::cout << "tokens:\n";
        }
        dumpTokens();
      } else {
        writeTokenFile();
      }
      return 0;
    }

    parser::Parser parser(source_name, tokens, diagnoser);
    parser.parse();

    if (diagnoser.has_error()) {
      std::cerr << diagnoser;
      return 1;
    }

    auto writeParseTreeFile = [&]() {
      const std::string ptree_path = source_name + ".ptree";
      auto ptree_file = openOutputFile(ptree_path);
      parser.print(*ptree_file);
    };

    auto dumpParseTree = [&]() { parser.print(std::cout, ascii); };

    if (mode == RunMode::Parser) {
      if (dump) {
        dumpParseTree();
      } else if (dump_all) {
        std::cout << "tokens:\n";
        dumpTokens();
        std::cout << "\nparse tree:\n";
        dumpParseTree();
      } else {
        writeTokenFile();
        writeParseTreeFile();
      }

      return 0;
    }

    semantic::SemanticAnalyzer analyzer(parser.program(), diagnoser);
    analyzer.analyze();

    if (!diagnoser.has_error() && !dump && !dump_all) {
      CodeGenerator icg(analyzer.getSymbolTable());
      analyzer.getAst().accept(icg);

      std::cout << "\n=== INTERMEDIATE CODE ===\n";
      icg.printCode();

      std::cout << "\n=== EXECUTION OUTPUT ===\n";
      Interpreter vm;
      if (!vm.execute(icg.getCode())) {
        return 1;
      }
    }
    
    if (diagnoser.has_error()) {
      std::cerr << diagnoser;
      return 1;
    }

    auto writeAstFile = [&]() {
      const std::string ast_path = source_name + ".ast";
      auto ast_file = openOutputFile(ast_path);
      semantic::ASTPrinter printer(*ast_file);
      printer.print(analyzer.getAst());
    };

    auto writeSymTabFile = [&]() {
      const std::string tab_path = source_name + ".sym.tab";
      auto tab_file = openOutputFile(tab_path);
      analyzer.getSymbolTable().printTab(*tab_file);
    };

    auto writeSymAtabFile = [&]() {
      const std::string atab_path = source_name + ".sym.atab";
      auto atab_file = openOutputFile(atab_path);
      analyzer.getSymbolTable().printAtab(*atab_file);
    };

    auto writeSymBtabFile = [&]() {
      const std::string btab_path = source_name + ".sym.btab";
      auto btab_file = openOutputFile(btab_path);
      analyzer.getSymbolTable().printBtab(*btab_file);
    };

    auto dumpAst = [&]() {
      semantic::ASTPrinter printer(std::cout, ascii);
      printer.print(analyzer.getAst());
    };

    auto dumpSemanticTables = [&]() {
      std::cout << "sym tab:\n";
      std::cout << "tab:\n";
      analyzer.getSymbolTable().printTab(std::cout);
      std::cout << "\nbtab:\n";
      analyzer.getSymbolTable().printBtab(std::cout);
      std::cout << "\natab:\n";
      analyzer.getSymbolTable().printAtab(std::cout);
    };

    if (dump) {
      std::cout << "ast:\n";
      dumpAst();
      std::cout << '\n';
      dumpSemanticTables();
    } else if (dump_all) {
      std::cout << "tokens:\n";
      dumpTokens();
      std::cout << "\nparse tree:\n";
      dumpParseTree();
      std::cout << "\nast:\n";
      dumpAst();
      std::cout << '\n';
      dumpSemanticTables();
    } else {
      writeTokenFile();
      writeParseTreeFile();
      writeAstFile();
      writeSymTabFile();
      writeSymAtabFile();
      writeSymBtabFile();
    }
    return 0;

  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}


