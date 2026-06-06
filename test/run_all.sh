#!/usr/bin/env bash

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
ARION="$ROOT/build/arion"

skip_build=0
if [[ "${1:-}" == "--skip-build" ]]; then
  skip_build=1
fi

if [[ "$skip_build" -eq 0 ]]; then
  cmake --build "$ROOT/build" || exit $?
fi

if [[ ! -x "$ARION" ]]; then
  echo "FAIL arion executable not found at $ARION"
  exit 1
fi

passed=0
failed=0

normalize() {
  sed 's/\r$//' | sed '/^[[:space:]]*$/d'
}

extract_execution_output() {
  awk '
    /^=== EXECUTION OUTPUT ===$/ { capture = 1; next }
    capture { print }
  ' | normalize
}

pass() {
  echo "PASS $1"
  passed=$((passed + 1))
}

fail() {
  echo "FAIL $1"
  failed=$((failed + 1))
}

run_stdout_case() {
  local source="$1"
  local expected="$2"
  local name
  name="$(basename "$source")"

  local output
  output="$("$ARION" "$source" 2>&1)"
  local exit_code=$?
  local actual expected_text
  actual="$(printf '%s\n' "$output" | extract_execution_output)"
  expected_text="$(normalize < "$expected")"

  if [[ "$exit_code" -eq 0 && "$actual" == "$expected_text" ]]; then
    pass "$name"
  else
    fail "$name (exit=$exit_code)"
    printf '%s\n' "$output"
  fi
}

run_stderr_case() {
  local source="$1"
  local expected="$2"
  local name
  name="$(basename "$source")"

  local output
  output="$("$ARION" "$source" 2>&1)"
  local exit_code=$?
  local expected_text
  expected_text="$(normalize < "$expected")"

  if [[ "$exit_code" -ne 0 && "$output" == *"$expected_text"* ]]; then
    pass "$name"
  else
    fail "$name (exit=$exit_code)"
    printf '%s\n' "$output"
  fi
}

run_ir_case() {
  local source="$SCRIPT_DIR/range_ok.pas"
  local ir_file="$source.ir"
  rm -f "$ir_file"

  local output
  output="$("$ARION" --ir "$source" 2>&1)"
  local exit_code=$?

  if [[ "$exit_code" -eq 0 && -s "$ir_file" &&
        "$output" != *"=== EXECUTION OUTPUT ==="* ]]; then
    pass "--ir writes IR without executing"
  else
    fail "--ir writes IR without executing (exit=$exit_code)"
    printf '%s\n' "$output"
  fi

  # Keep the generated IR file around so it can be inspected after the run.
}

run_dump_ir_case() {
  local source="$SCRIPT_DIR/range_ok.pas"
  local output
  output="$("$ARION" --ir --dump "$source" 2>&1)"
  local exit_code=$?
  local actual
  actual="$(printf '%s\n' "$output" | extract_execution_output)"

  if [[ "$exit_code" -eq 0 &&
        "$output" == *"=== DUMP: IR ==="* &&
        "$output" == *"=== EXECUTION OUTPUT ==="* &&
        "$actual" == "5" ]]; then
    pass "--ir --dump separates IR dump from execution output"
  else
    fail "--ir --dump separates IR dump from execution output (exit=$exit_code)"
    printf '%s\n' "$output"
  fi
}

for expected in "$SCRIPT_DIR"/*.expected.out; do
  source="${expected%.expected.out}"
  [[ -f "$source" ]] || continue
  run_stdout_case "$source" "$expected"
done

for expected in "$SCRIPT_DIR"/*.expected.err; do
  source="${expected%.expected.err}"
  [[ -f "$source" ]] || continue
  run_stderr_case "$source" "$expected"
done

run_ir_case
run_dump_ir_case

echo
echo "Summary: $passed passed, $failed failed"
if [[ "$failed" -gt 0 ]]; then
  exit 1
fi
exit 0
