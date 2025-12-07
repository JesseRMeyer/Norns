#include "../../third_party/fast_math.hpp"

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

