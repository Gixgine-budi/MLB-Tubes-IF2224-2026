#pragma once

#include <string>
#include <vector>

#include "diagnoser/diagnoser.hpp"
#include "ir/instruction.hpp"

namespace vm {

/**
 * @brief Executes stack-machine IR produced by ir::CodeGenerator.
 *
 * The interpreter owns the VM stack and runtime registers (`pc`, `b`, `t`) for
 * a single execution. Runtime failures are reported through the shared
 * diagnoser instead of being printed directly, so they use the same diagnostic
 * path as lexer, parser, and semantic errors.
 */
class Interpreter {
 public:
  /**
   * @brief Construct a VM interpreter with a diagnostic sink.
   *
   * @param diagnoser Diagnostic accumulator used for runtime errors. It must
   * outlive the interpreter.
   */
  explicit Interpreter(diag::Diagnoser& diagnoser);

  /**
   * @brief Execute an IR program until RET from the main frame or failure.
   *
   * @param code Generated instruction sequence
   * @return true when execution completes successfully, false after reporting a
   * VM diagnostic
   */
  bool execute(const std::vector<ir::Instruction>& code);

 private:
  static constexpr int kMaxStack = 10000;
  static constexpr int kMaxCallDepth = 1000;

  /**
   * @brief Report a runtime diagnostic and stop execution.
   */
  bool fail(const std::string& code, const std::string& detail = "");

  /**
   * @brief Verify that at least one stack value is available.
   */
  bool checkStackPop();

  /**
   * @brief Verify a jump target points inside the instruction stream.
   */
  bool checkJumpTarget(int target, int code_size);

  /**
   * @brief Build a descriptive VM diagnostic message from an internal code.
   */
  std::string runtimeMessage(const std::string& code,
                             const std::string& detail) const;

  /**
   * @brief Build an optional user-facing hint for a runtime error code.
   */
  std::string runtimeHint(const std::string& code) const;

  /**
   * @brief Follow static links to find the base address for lexical level `l`.
   */
  int getBase(int l);

  diag::Diagnoser& diagnoser_;
  std::vector<int> s;  ///< VM stack storage
  int pc;             ///< Program counter
  int current_instruction;  ///< Instruction address currently executing
  int b;                    ///< Current frame base pointer
  int t;                    ///< Top-of-stack index
  int call_depth;           ///< Guard against runaway recursion
};

}  // namespace vm
