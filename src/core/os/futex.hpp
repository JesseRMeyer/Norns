#pragma once

namespace os {

class Futex {
public:
	Futex(): s(new storage{}) {}

	Futex(Futex& other): s(other.s) {}

	Futex(Futex&& other): s(exchange(other.s, {})) {}

	~Futex() {
		if (s == nullptr) {
			return;
		}

		if (s->lock == LOCKED) {
			Unlock();
		}

		delete s;
		s = nullptr;
	}

	void 
	Lock() {
		_Wait();
	}

	bool 
	TryLock() {
		if (s == nullptr) {
			return false;
		}

		if (s->lock == LOCKED) {
			return false;
		}

		Lock();
		return true;
	}

	void 
	Unlock() {
		_Wake();
	}

	template <typename U>
	Futex&
	operator=(U&& other) {
		s = forward<U>(other->s);
		return *this;
	}

private:
	//TODO(Jesse): Add waiters field to elide unnecessary _Wake() when there are no waiters.
	// currently Unlock() always incurs a syscall.

	//NOTE(Jesse): Keep variable on its own cache line and aligned to reduce
	// false sharing / contention.
	// Using a struct with 15 i32 padding elements causes GCC to throw a pedantic fit.
	union alignas(64) storage {
		atomic<i32> lock = UNLOCKED;
		byte cache_line[64];
	};

	storage* s;

	constexpr internal i32 UNLOCKED = 0;
	constexpr internal i32 LOCKED = 1;

	#include PLATFORM_CPP(futex/futex)
};
}