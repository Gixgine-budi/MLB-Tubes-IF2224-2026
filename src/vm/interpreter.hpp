#pragma once

#include <string>
#include <vector>

#include "diagnoser/diagnoser.hpp"
#include "ir/instruction.hpp"

namespace vm {

class Interpreter {
 public:
  explicit Interpreter(diag::Diagnoser& diagnoser);

  bool execute(const std::vector<ir::Instruction>& code);

 private:
  static constexpr int kMaxStack = 10000;
  static constexpr int kMaxCallDepth = 1000;

  bool fail(const std::string& code, const std::string& detail = "");
  bool checkStackPop();
  bool checkJumpTarget(int target, int code_size);
  std::string runtimeMessage(const std::string& code,
                             const std::string& detail) const;
  std::string runtimeHint(const std::string& code) const;
  int getBase(int l);

  diag::Diagnoser& diagnoser_;
  std::vector<int> s;
  int pc;
  int current_instruction;
  int b;
  int t;
  int call_depth;
};

}  // namespace vm
