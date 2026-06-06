#pragma once

#include <string>

namespace ir {

/**
 * @brief Operation codes for the stack-machine intermediate representation.
 *
 * The VM treats instructions as PL/0-like triples plus an auxiliary field.
 */
enum class OpCode {
  LIT,  // Load literal value
  LOD,  // Load variable from stack frame
  STO,  // Store variable to stack frame
  CAL,  // Call procedure
  INT,  // Increment stack pointer for procedure entry
  JMP,  // Unconditional jump
  JPC,  // Conditional jump (jump if top of stack is zero)
  OPR,  // Perform arithmetic or logical operation (subcode in `a`)
  RET,  // Return from procedure
  LDX,  // Load from array with bounds checking (aux = array size)
  STX,  // Store to array with bounds checking (aux = array size)
  CHK   // Check array bounds (aux = array size)
};

/**
 * @brief One generated IR instruction consumed by the VM interpreter.
 *
 * `aux` is reserved for opcodes that need a fourth operand, currently array
 * load/store bounds. Unused auxiliary values are kept as zero so printed IR
 * remains compact.
 */
struct Instruction {
  OpCode op;  // Operation code
  int l;      // Lexical level distance
  int a;      // Address, literal value, jump target, or operator subcode
  int aux;    // Auxiliary field for opcodes that need a fourth operand (e.g.
              // array bounds)

  /**
   * @brief Return the mnemonic used when printing this instruction.
   *
   * @return std::string opcode name
   */
  std::string toString() const {
    switch (op) {
      case OpCode::LIT:
        return "LIT";
      case OpCode::LOD:
        return "LOD";
      case OpCode::STO:
        return "STO";
      case OpCode::CAL:
        return "CAL";
      case OpCode::INT:
        return "INT";
      case OpCode::JMP:
        return "JMP";
      case OpCode::JPC:
        return "JPC";
      case OpCode::OPR:
        return "OPR";
      case OpCode::RET:
        return "RET";
      case OpCode::LDX:
        return "LDX";
      case OpCode::STX:
        return "STX";
      case OpCode::CHK:
        return "CHK";
      default:
        return "???";
    }
  }
};

}  // namespace ir
