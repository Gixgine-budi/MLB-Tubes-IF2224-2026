#include "vm/interpreter.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "ir/instruction.hpp"

namespace vm {

Interpreter::Interpreter()
    : s(kMaxStack, 0), pc(0), b(0), t(-1), call_depth(0) {}

bool Interpreter::fail(const std::string& message) {
  std::cerr << message << "\n";
  return false;
}

bool Interpreter::checkStackPop() {
  if (t < 0) {
    return fail("StackUnderflowError");
  }
  return true;
}

bool Interpreter::checkJumpTarget(int target, int code_size) {
  if (target < 0 || target >= code_size) {
    return fail("InvalidJumpTarget");
  }
  return true;
}

int Interpreter::getBase(int l) {
  int b1 = b;
  while (l > 0) {
    if (b1 < 0 || b1 >= kMaxStack) {
      return 0;
    }
    b1 = s[b1];
    l--;
  }
  return b1;
}

bool Interpreter::execute(const std::vector<ir::Instruction>& code) {
  if (code.empty()) {
    return true;
  }

  ir::Instruction i;
  do {
    if (pc < 0 || pc >= static_cast<int>(code.size())) {
      return fail("InvalidJumpTarget");
    }

    i = code[pc++];

    switch (i.op) {
      case ir::OpCode::LIT:
        if (t + 1 >= kMaxStack) {
          return fail("StackOverflowError");
        }
        s[++t] = i.a;
        break;
      case ir::OpCode::LOD: {
        if (i.a < 0) {
          if (b + i.a < 0 || b + i.a >= kMaxStack) {
            return fail("StackAccessError");
          }
          s[++t] = s[b + i.a];
        } else {
          const int base = getBase(i.l);
          if (base + i.a < 0 || base + i.a >= kMaxStack) {
            return fail("StackAccessError");
          }
          s[++t] = s[base + i.a];
        }
        break;
      }
      case ir::OpCode::STO: {
        if (!checkStackPop()) return false;
        if (i.a < 0) {
          if (b + i.a < 0 || b + i.a >= kMaxStack) {
            return fail("StackAccessError");
          }
          s[b + i.a] = s[t--];
        } else {
          const int base = getBase(i.l);
          if (base + i.a < 0 || base + i.a >= kMaxStack) {
            return fail("StackAccessError");
          }
          s[base + i.a] = s[t--];
        }
        break;
      }
      case ir::OpCode::CAL: {
        if (!checkJumpTarget(i.a, static_cast<int>(code.size()))) {
          return false;
        }
        if (call_depth >= kMaxCallDepth) {
          return fail("StackOverflowError");
        }
        if (t + 3 >= kMaxStack) {
          return fail("StackOverflowError");
        }
        s[t + 1] = getBase(i.l);
        s[t + 2] = b;
        s[t + 3] = pc;
        b = t + 1;
        pc = i.a;
        ++call_depth;
        break;
      }
      case ir::OpCode::INT:
        if (t + i.a >= kMaxStack) {
          return fail("StackOverflowError");
        }
        t = t + i.a;
        break;
      case ir::OpCode::JMP:
        if (!checkJumpTarget(i.a, static_cast<int>(code.size()))) {
          return false;
        }
        pc = i.a;
        break;
      case ir::OpCode::JPC:
        if (!checkStackPop()) return false;
        if (s[t--] == 0) {
          if (!checkJumpTarget(i.a, static_cast<int>(code.size()))) {
            return false;
          }
          pc = i.a;
        }
        break;
      case ir::OpCode::OPR:
        switch (i.a) {
          case 1:
            if (!checkStackPop()) return false;
            s[t] = -s[t];
            break;
          case 2:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = s[t] + s[t + 1];
            break;
          case 3:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = s[t] - s[t + 1];
            break;
          case 4:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = s[t] * s[t + 1];
            break;
          case 5:
            if (t < 1) return fail("StackUnderflowError");
            if (s[t] == 0) return fail("DivisionByZeroError");
            t--;
            s[t] = s[t] / s[t + 1];
            break;
          case 6:
            if (t < 1) return fail("StackUnderflowError");
            if (s[t] == 0) return fail("DivisionByZeroError");
            t--;
            s[t] = s[t] % s[t + 1];
            break;
          case 7:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] == s[t + 1]) ? 1 : 0;
            break;
          case 8:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] != s[t + 1]) ? 1 : 0;
            break;
          case 9:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] < s[t + 1]) ? 1 : 0;
            break;
          case 10:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] >= s[t + 1]) ? 1 : 0;
            break;
          case 11:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] > s[t + 1]) ? 1 : 0;
            break;
          case 12:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] <= s[t + 1]) ? 1 : 0;
            break;
          case 13:
            if (!checkStackPop()) return false;
            std::cout << s[t--];
            break;
          case 14:
            std::cout << "\n";
            break;
          case 15:
            if (!checkStackPop()) return false;
            s[t] = s[t] == 0 ? 1 : 0;
            break;
          case 16:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] != 0 && s[t + 1] != 0) ? 1 : 0;
            break;
          case 17:
            if (t < 1) return fail("StackUnderflowError");
            t--;
            s[t] = (s[t] != 0 || s[t + 1] != 0) ? 1 : 0;
            break;
        }
        break;
      case ir::OpCode::RET: {
        int ret_val = 0;
        if (t >= 0) {
          ret_val = s[t];
        }
        if (b == 0) {
          t = -1;
          pc = 0;
          break;
        }
        t = b - 1;
        pc = s[b + 2];
        b = s[b + 1];
        if (call_depth > 0) {
          --call_depth;
        }
        if (t + 1 < kMaxStack) {
          s[++t] = ret_val;
        }
        break;
      }
      case ir::OpCode::LDX: {
        if (t < 0) return fail("StackUnderflowError");
        int rel = s[t--];
        if (rel < 0 || rel >= i.aux) {
          return fail("IndexOutOfBoundsException");
        }
        const int base = getBase(i.l);
        if (base + i.a + rel < 0 || base + i.a + rel >= kMaxStack) {
          return fail("StackAccessError");
        }
        s[++t] = s[base + i.a + rel];
        break;
      }
      case ir::OpCode::STX: {
        if (t < 1) return fail("StackUnderflowError");
        int val = s[t--];
        int rel = s[t--];
        if (rel < 0 || rel >= i.aux) {
          return fail("IndexOutOfBoundsException");
        }
        const int base = getBase(i.l);
        if (base + i.a + rel < 0 || base + i.a + rel >= kMaxStack) {
          return fail("StackAccessError");
        }
        s[base + i.a + rel] = val;
        break;
      }
      case ir::OpCode::CHK:
        if (!checkStackPop()) return false;
        if (s[t] < i.l || s[t] > i.a) {
          return fail("RangeCheckError");
        }
        break;
    }
  } while (pc != 0);

  return true;
}

}  // namespace vm
