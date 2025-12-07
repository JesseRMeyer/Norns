#include "fast_math_internal.hpp"

// ---- https://github.com/nadavrot/fast_log ---- //
/*
MIT License

Copyright (c) 2022 Nadav Rotem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

internal f32
exp(f32 x) {
    f32 integer = (f32)(i32)x;
    x = x - integer;

    // Use a 4-part polynomial to approximate exp(x);
    f32 c[4] = {0.28033708f, 0.425302f, 1.01273643f, 1.00020947f};

    // Use Horner's method to evaluate the polynomial.
    f32 val = c[3] + x * (c[2] + x * (c[1] + x * (c[0])));
    return val * EXP_TABLE[(u32)integer + 710];
}

internal f32 
ln(f32 x) {
	f32 const epsilon = 1e-10f;
	f32 yn = x - 1.0f;
	f32 yn1 = yn;

	do {
		yn = yn1;
		yn1 = yn + 2 * (x - exp(yn)) / (x + exp(yn));
	} while (abs(yn - yn1) > epsilon);

	return yn1;
}

struct FrExPResult {
	f64 f;
	i32 i;
};

FrExPResult internal 
frexp(f64 x) {
    u64 bits = *(u64*)&x;
    if (bits == 0) {
        return { 0., 0 };
    }
    // See:
    // https://en.wikipedia.org/wiki/IEEE_754#Basic_and_interchange_formats

    // Extract the 52-bit mantissa field.
    u64 mantissa = bits & 0xFFFFFFFFFFFFF;
    bits >>= 52;

    // Extract the 11-bit exponent field, and add the bias.
    i32 exponent = i32(bits & 0x7ff) - 1023;
    bits >>= 11;

    // Extract the sign bit.
    u64 sign = bits;
    bits >>= 1;

    // Construct the normalized double;
    u64 res = sign;
    res <<= 11;
    res |= 1023 - 1;
    res <<= 52;
    res |= mantissa;

    f64 frac = *(f64*)&res;
    return { frac, exponent + 1 };
}

f32 log(f32 x) {
    /// Extract the fraction, and the power-of-two exponent.

    auto a = frexp((f64)x);
    x = a.f;
    i32 pow2 = a.i;

    // Use a 4-part polynom to approximate log2(x);
    f32 c[] = { 1.33755322, -4.42852392, 6.30371424, -3.21430967 };
    f32 log2 = 0.6931471805599453;

    // Use Horner's method to evaluate the polynomial.
    f32 val = c[3] + x * (c[2] + x * (c[1] + x * (c[0])));

    // Compute log2(x), and convert the result to base-e.
    return log2 * (pow2 + val);
}
