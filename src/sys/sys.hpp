//NOTE(Jesse): Prefer C headers for faster compilation times, except where semantics vitally differ.

#include <stdint.h>
#include <float.h>
#include <atomic> //NOTE(Jesse): We want C++ headers here for proper atomic semantics (without needing to explicitly annotate atomic operations with functions)
#include <stdio.h>
#include <utility> //NOTE(Jesse): std::forward

#include <cstddef> //NOTE(Jesse): std::max_align_t
