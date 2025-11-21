template<class T>
const T& min(const T& a, const T& b) {
    return a <= b ? a : b;
}

template<class T>
const T& max(const T& a, const T& b) {
    return a >= b ? a : b;
}

template<class T>
const T& abs(const T& a) {
    return a >= 0 ? a : -a;
}

internal inline
size_t align(uintptr_t p, uintptr_t alignment) {
	return (alignment - (p & (alignment - 1))) & (alignment - 1);
}

/*
internal void
mem_copy(byte* dst, byte* src, u32 byte_count) {
	if (not dst or not src or not byte_count) {
		return;
	}

    if (unlikely(byte_count < 8)) {
        while (byte_count--) {
        	*dst++ = *src++;
        }
        
        return;
    }

    //NOTE(Jesse): Push out single bytes until we reach 64 byte alignment.
    size_t bytes_left_to_align = align((uintptr_t)dst, 64);
    byte_count -= bytes_left_to_align;
    while (bytes_left_to_align-- > 0) {
    	*dst++ = *src++;
    }

    assert((size_t)dst % 64 == 0);

    const size_t block_byte_count = sizeof(u64);
	size_t block_count = byte_count / block_byte_count;    
	for (size_t block_idx = 0; block_idx < block_count; ++block_idx) {
        //TODO(Jesse): SIMD.
        *(u64*)dst = *(u64*)src;
        src += block_byte_count;
        dst += block_byte_count;
    }

    byte_count -= block_count * block_byte_count;
    while (byte_count-- > 0) {
    	*dst++ = *src++;
    }
}
*/
