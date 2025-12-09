#include "../../third_party/fast_math.hpp"

union V2 {
	struct {
		f32 x;
		f32 y;
	};

	f32 e[2];

	V2
	operator+(f32 other) {
		return {x + other, y + other};
	}

	V2
	operator*(f32 other) {
		return {x * other, y * other};
	}

	V2
	operator/(f32 other) {
		other = 1.0f / other;
		return {x * other, y * other};
	}

	V2&
	operator+=(V2& other) {
		x += other.x; 
		y += other.y;
		return *this;
	}

	V2&
	operator*=(V2& other) {
		x *= other.x; 
		y *= other.y;
		return *this;
	}

	template <typename U>
	V2&
	operator*=(U&& other) {
		x *= other; 
		y *= other;
		return *this;
	}

	friend StringStream&
	operator<<(StringStream& s, V2 v) {
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
		#pragma clang diagnostic ignored "-Wformat-extra-args"

		char buffer[32] = {};
		int characters_written = stbsp_snprintf((char*)&buffer[0], size(buffer), "V2: y: %_$$$g, x: %_$$$g", (f64)v.y, (f64)v.x);
		(void)characters_written;
		s << (char*)buffer;

		#pragma clang diagnostic pop

		return s;
	}
};

V2
operator+(V2 a, V2 b) {
	return {a.x + b.x, a.y + b.y};
}

V2
operator-(V2 a, V2 b) {
	return {a.x - b.x, a.y - b.y};
}

f32
sqrt(f32 x) { //NOTE(Jesse): Heron's method
	(void)ln; //NOTE(Jesse): Ignore - gets rid of "unused function" warning.

	f32 const epsilon = 1e-10f;
	assert(x >= 0.0f);
	
	if (x == 0.0f) {
		return 0.0f;
	}
	
	f32 guess = x;
	if (guess < 1.0f) {
		guess = 1.0f;
	}
	
	while (true) {
		f32 next_guess = (guess + x / guess) * 0.5f;
		if (abs(next_guess - guess) < epsilon) {
			return next_guess;
		}

		guess = next_guess;
	}
}

