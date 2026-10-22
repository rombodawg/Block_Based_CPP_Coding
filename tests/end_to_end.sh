#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 2 ]]; then
  echo "usage: end_to_end.sh <CppCircuitLab executable> <C++ compiler>" >&2
  exit 2
fi
APP="$1"
CXX="$2"
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

"$APP" --self-test
"$APP" --emit-demo "$WORK/starter.cpp"
"$CXX" -std=c++17 -Wall -Wextra -pedantic "$WORK/starter.cpp" -o "$WORK/starter"
"$WORK/starter" > "$WORK/output.txt"
cat > "$WORK/expected.txt" <<'EXPECTED'
Change the Number tile and run again.
The score is zero.
EXPECTED
diff -u "$WORK/expected.txt" "$WORK/output.txt"
echo "[PASS] generated starter code compiles and runs with the expected output"

"$CXX" -std=c++17 -Wall -Wextra -pedantic -I"$PROJECT_DIR/src" "$PROJECT_DIR/tests/string_literal_source.cpp" -o "$WORK/string_generator"
"$WORK/string_generator" > "$WORK/escaped_message.cpp"
"$CXX" -std=c++17 -Wall -Wextra -pedantic "$WORK/escaped_message.cpp" -o "$WORK/escaped_message"
"$WORK/escaped_message" > "$WORK/escaped_output.txt"
cat > "$WORK/escaped_expected.txt" <<'EXPECTED'
first line
quotes: "hello" path: C:\temp
EXPECTED
diff -u "$WORK/escaped_expected.txt" "$WORK/escaped_output.txt"
echo "[PASS] generated source safely handles embedded newlines, quotes, and backslashes"

"$CXX" -std=c++17 -Wall -Wextra -pedantic -I"$PROJECT_DIR/src" "$PROJECT_DIR/tests/value_slots_source.cpp" -o "$WORK/value_slots_generator"
"$WORK/value_slots_generator" > "$WORK/value_slots.cpp"
"$CXX" -std=c++17 -Wall -Wextra -pedantic "$WORK/value_slots.cpp" -o "$WORK/value_slots"
"$WORK/value_slots" > "$WORK/value_slots_output.txt"
cat > "$WORK/value_slots_expected.txt" <<'EXPECTED'
43
Text value is reusable!
EXPECTED
diff -u "$WORK/value_slots_expected.txt" "$WORK/value_slots_output.txt"
echo "[PASS] reusable Type name, Name, Number, Expression, Text, and Character tiles generate and run real C++"

"$CXX" -std=c++17 -Wall -Wextra -pedantic -I"$PROJECT_DIR/src" "$PROJECT_DIR/tests/advanced_blocks_source.cpp" -o "$WORK/advanced_generator"
"$WORK/advanced_generator" > "$WORK/advanced.cpp"
"$CXX" -std=c++17 -Wall -Wextra -pedantic "$WORK/advanced.cpp" -o "$WORK/advanced"
printf '7\n' | "$WORK/advanced" > "$WORK/advanced_output.txt"
cat > "$WORK/advanced_expected.txt" <<'EXPECTED'
4
7
2
EXPECTED
diff -u "$WORK/advanced_expected.txt" "$WORK/advanced_output.txt"
echo "[PASS] generated advanced C++ compiles and runs with input, vectors, for/while loops, continue and break"
