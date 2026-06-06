#pragma once

#include <string>
#include <vector>

#include "ir/instruction.hpp"

namespace vm {

class Interpreter {
 public:
  Interpreter();

  bool execute(const std::vector<ir::Instruction>& code);

 private:
  static constexpr int kMaxStack = 10000;
  static constexpr int kMaxCallDepth = 1000;

  bool fail(const std::string& message);
  bool checkStackPop();
  bool checkJumpTarget(int target, int code_size);
  int getBase(int l);

  std::vector<int> s;
  int pc;
  int b;
  int t;
  int call_depth;
};

}  // namespace vm
