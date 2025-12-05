#define _STR(x) #x
#define STR(x) _STR(x)
#define CONCAT(x, y) x##y

#ifdef _WIN32
	#define WINDOWS
	#define PLATFORM_CPP(fn) STR(CONCAT(fn, _windows.cpp))
	#define PLATFORM_H(fn) STR(CONCAT(fn, _windows.hpp))

#elif defined(__linux__)
	#define LINUX
	#define PLATFORM_CPP(fn) STR(CONCAT(fn, _linux.cpp))
	#define PLATFORM_H(fn) STR(CONCAT(fn, _linux.hpp))

#elif defined(__APPLE__) && defined(__MACH__)
	#define OSX
	#define PLATFORM_CPP(fn) STR(CONCAT(fn, _osx.cpp))
	#define PLATFORM_H(fn) STR(CONCAT(fn, _osx.hpp))

#endif

#ifdef __clang__
	#define COMPILER_CLANG 1
	#define COMPILER_NAME "Clang"
#elif defined(__GNUC__) || defined(__GNUG__)
	#define COMPILER_GCC 1
	#define COMPILER_NAME "GCC"
#elif defined(_MSC_VER)
	#define COMPILER_MSVC 1
	#define COMPILER_NAME "MSVC"
#elif defined(__INTEL_COMPILER)
	#define COMPILER_INTEL 1
	#define COMPILER_NAME "Intel"
#else
	#define COMPILER_UNKNOWN 1
	#define COMPILER_NAME "Unknown"
#endif

//NOTE(Jesse): For reasons I do not know
// using defined(__SANITIZE_ADDRESS__) or defined(__SANITIZE_THREAD__)
// results in limited code-gen for either.  For example, the __asan_poison_memory_region
// calls are elided.  Weird undefined behavior?

#if ASAN
  extern "C" {
    void __asan_poison_memory_region(void const volatile*, size_t);
    void __asan_unpoison_memory_region(void const volatile*, size_t);
  }

  #define ASAN_POISON(addr, size) __asan_poison_memory_region((addr), (size))
  #define ASAN_UNPOISON(addr, size) __asan_unpoison_memory_region((addr), (size))
  #define ASAN_IGNORE __attribute__((no_sanitize_address))

#else

  #define ASAN_POISON(addr, size)
  #define ASAN_UNPOISON(addr, size)
  #define ASAN_IGNORE

#endif

#ifdef COMPILER_CLANG
#define assume(cond) __builtin_assume(cond)
#else
#define assume(cond)
#endif

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

[[gnu::noinline]]
void __attribute__((cold))
_assert_failed(const char* file, const char* pretty_func, u32 line_number, const char* cond) {
	fprintf(stderr, "[ASSERT]: %s: %s: %u: %s\n", file, pretty_func, line_number, cond);
	*(u8 volatile *)0 = 0;
}

//NOTE(Jesse): Separate _assert_failed() function to prohibit cpu speculation.
#define assert(cond) if (unlikely(not (cond))) {_assert_failed(__FILE__, __PRETTY_FUNCTION__, __LINE__, #cond); \
} assume(cond)

template <typename T>
void 
swap(T& a, T& b) {
    T temp = move(a);
    a = move(b);
    b = move(temp);
}

//NOTE(Jesse): Relies on C++17 move semantic optimization defaults, otherwise we'd apparently need to wrap this.
template <typename F>
struct _defer {
	F f;
	_defer(F f): f(f) {}
	~_defer() { f(); }
};

#define __DEFER(x, y) CONCAT(x, y)
#define _DEFER(x) __DEFER(x, __COUNTER__)
#define defer(code) auto _DEFER(_defer_) = _defer([&](){code;})

#define KILOBYTES(x) (1024 * (x))
#define MEGABYTES(x) (KILOBYTES(1024) * (x))
#define GIGABYTES(x) (MEGABYTES(1024) * (x))

#include PLATFORM_H(includes/includes)
#include PLATFORM_H(base/base)
