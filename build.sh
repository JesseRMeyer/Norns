#!/bin/bash
# Usage: optimized|debug [analyze] [run]

#https://gist.github.com/mohanpedala/1e2ff5661761d3abd0385e8223e16425?permalink_comment_id=3935570
set -euo pipefail

BUILD_TYPE="debug"
ANALYZE=false
RUN=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    optimized|debug) BUILD_TYPE="$1" ;;
    analyze)       ANALYZE=true ;;
    run)           RUN=true     ;;
    *)             echo "Unknown option: $1" >&2; exit 1 ;;
  esac
  shift
done

if [[ -z "$BUILD_TYPE" ]]; then
  echo "Usage: $0 <optimized|debug> [analyze] [run]" >&2
  exit 1
fi

rm -f build/Norns_v3 build/Norns_Test_v3

OPT_FLAGS=""
case "$BUILD_TYPE" in
  optimized) OPT_FLAGS=" -O2 -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -fno-stack-check" ;;
  debug)     OPT_FLAGS=" -O0 -g3 -fno-omit-frame-pointer" ;;
esac

WARN_FLAGS=" -Wall -Wextra -Wno-missing-braces -Wno-narrowing -Wswitch-enum -Wno-assume -Wdouble-promotion -Wno-unqualified-std-cast-call"
BEHAVIOR_FLAGS="-std=gnu++20 -Wno-write-strings -fno-exceptions -fwrapv -fno-implicit-templates -fno-delete-null-pointer-checks -ftrivial-auto-var-init=zero"
MATH_FLAGS=" -fno-math-errno -fcx-limited-range -fno-signed-zeros -fno-trapping-math -freciprocal-math -ffinite-math-only"

LIBS=" -lxcb -lxcb-keysyms -lxcb-present -lxcb-util -lxcb-shm -pthread"

COMPILER="clang++"
PRIMARY_SRC_FILE="src/main.cpp"
BASE_BUILD_COMMAND=" $PRIMARY_SRC_FILE $WARN_FLAGS $BEHAVIOR_FLAGS $MATH_FLAGS $OPT_FLAGS $LIBS -o build/Norns_v3 -flto -fuse-ld=lld -march=x86-64-v3" #-DASAN -fsanitize=address -DTSAN -fsanitize=thread"
CLANG_BUILD_COMMAND="$COMPILER $BASE_BUILD_COMMAND"

GCC_IGNORE_WARNINGS=" -Wno-unknown-pragmas -Wno-format"
GCC_CHECK_COMMAND="g++ $BASE_BUILD_COMMAND $GCC_IGNORE_WARNINGS -fsyntax-only"

$CLANG_BUILD_COMMAND &

#Optionally enable GCC error checking.  GCC is more pedantic than Clang.
$GCC_CHECK_COMMAND

PRIMARY_TEST_FILE="src/test/test.cpp"
TEST_BUILD_COMMAND=" $PRIMARY_TEST_FILE $WARN_FLAGS $BEHAVIOR_FLAGS $MATH_FLAGS $OPT_FLAGS $LIBS -o build/Norns_Test_v3 -flto -fuse-ld=lld -march=x86-64-v3 -Isrc -DASAN -fsanitize=address,undefined"
CLANG_BUILD_COMMAND="$COMPILER $TEST_BUILD_COMMAND"

$CLANG_BUILD_COMMAND

./build/Norns_Test_v3

if $ANALYZE; then
  BASE_BUILD_COMMAND+=" --analyze"
  CLANG_ANALYZE_COMMAND="$COMPILER $BASE_BUILD_COMMAND"
  $CLANG_ANALYZE_COMMAND
fi

if $RUN; then
  ./build/Norns_v3
fi
