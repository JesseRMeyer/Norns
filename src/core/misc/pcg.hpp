#pragma once

struct pcg32_random_t {
	u64 state;  
	u64 inc; 
};

constexpr internal inline u32 
pcg32_random(pcg32_random_t& rng) {
	u64 oldstate = rng.state;
	// Advance internal state
	rng.state = oldstate * 6364136223846793005ull + (rng.inc | 1u);
	// Calculate output function (XSH RR), uses old state for max ILP
	u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	u32 rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31u));
}

//TODO(Jesse): Specialize for larger types.
constexpr internal inline u32 
pcg32_hash(byte* data, u32 bytes_size) {
	pcg32_random_t rng = {
		14695981039346656037ull,
		1099511628211ull,
	};

	u32 val = pcg32_random(rng);

	for(u32 b_idx = 0; b_idx < bytes_size; ++b_idx) {
		rng.state ^= data[b_idx];
		val ^= pcg32_random(rng);
	}

	//TODO(Jesse): Do 1 byte at a time until byte pointer is aligned to 8 with sufficient size remaining.

	return val;
}

//NOTE(Jesse): These values are taken from the FVNA hash initializations state
constexpr internal inline void
pcg32_init(pcg32_random_t& rng, u64 initial_sequence = 14695981039346656037ull, u64 initial_state = 1099511628211ull) {
	rng.state = 0;
	rng.inc = (initial_sequence << 1u) | 1u;

	pcg32_random(rng);

	rng.state += initial_state;
	pcg32_random(rng);
}