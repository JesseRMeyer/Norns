//NOTE(Jesse): https://en.wikipedia.org/wiki/Marsaglia_polar_method
template <typename RNGUni01>
f32
SampleGaussian(RNGUni01& rng_ctx, f32 mean, f32 std_dev) {
	global f32 spare;
	global bool spare_available = false;

	if (spare_available) {
		spare_available = false;
		return (spare * std_dev) + mean;
	} 

	f32 u, v, s;
	do {
		u = (f32)rng_ctx() * 2.0f - 1.0f;
		v = (f32)rng_ctx() * 2.0f - 1.0f;
		s = (u * u) + (v * v);
	} while (s >= 1.0f or s == 0.0f);

	s = sqrt(-2.0f * log(s) / s);
	spare = v * s;
	spare_available = true;
	return mean + (std_dev * u * s);
}

template <typename RNGUni01>
f32
SampleStandardGaussian(RNGUni01& rng_ctx) {
	global f32 spare;
	global bool spare_available = false;

	if (spare_available) {
		spare_available = false;
		return spare;
	} 

	f32 u, v, s;
	do {
		u = (f32)rng_ctx() * 2.0f - 1.0f;
		v = (f32)rng_ctx() * 2.0f - 1.0f;
		s = (u * u) + (v * v);
	} while (s >= 1.0f or s == 0.0f);

	s = sqrt(-2.0f * log(s) / s);
	spare = v * s;
	spare_available = true;
	return 	u * s;
}
