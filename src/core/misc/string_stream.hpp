#pragma once

#include "../os/futex.hpp"

//TODO(Jesse): Proper String Class.

class StringStream {
public:
	StringStream() = default;
	StringStream(StringStream& other) = delete;
	StringStream(StringStream&& other) = delete;

	StringStream& operator=(StringStream& other) = delete;
	StringStream& operator=(StringStream&& other) = delete;

	template <typename T>
	inline StringStream& 
	operator<<(T n) { //NOTE(Jesse): Convert most implictly convertable numerics to doubles.
		return operator<<(f64(n));
	}

	inline StringStream& 
	operator<<(char* str) {
		fprintf(stdout, "%s\n", str);

		return *this;
	}

	inline StringStream& 
	operator<<(const char* str) {
		fprintf(stdout, "%s\n", str);

		return *this;
	}

	inline StringStream& 
	operator<<(char character) {
		fprintf(stdout, "%c\n", character);

		return *this;
	}

	inline StringStream& 
	operator<<(bool cond) {
		fprintf(stdout, "%s\n", cond ? "True" : "False");

		return *this;
	}

	StringStream& 
	operator<<(f64 n) {
		//NOTE(Jesse): stb has a custom format string format that confuses both clang and gcc
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
		
		futex.Lock();

		int characters_written = stbsp_snprintf((char*)&buffer[0], capacity, "%_$$$g", n);
		fprintf(stdout, "%s\n", (char*)&buffer[0]);

		if (characters_written >= capacity) {
			//TODO(Jesse): stbsp_vsprintfcb to print entire message
			fprintf(stdout, "%s\n", "[NOTE]: Message exceeded message buffer of 4096 characters.");
		}

		futex.Unlock();

		#pragma clang diagnostic pop

		return *this;
	}

private:
	constexpr internal i32 capacity = 4096;
	struct alignas(4096) {
		byte buffer[capacity] = {};
	};

	os::Futex futex;
};