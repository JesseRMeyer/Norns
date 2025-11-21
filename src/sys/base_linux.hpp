inline internal void*
SYS_ACQUIRE_PLATFORM_VIRTUAL_MEMORY(u32 bytes_count) {
	#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
	#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

	int mmap_flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE;
	//TODO(Jesse): Why doesn't huge pages work?
	//if (bytes_count >= default_huge_page_byte_size) {
	//	mmap_flags |= (MAP_HUGETLB | MAP_HUGE_2MB);
	//}

	size_t total_bytes_count = bytes_count; 
	void* addr = mmap(nil, total_bytes_count, PROT_READ | PROT_WRITE, mmap_flags, -1, 0);
	if (unlikely((size_t)addr == (size_t)-1)) {
		fprintf(stderr, "[ERROR] madvise: %s\n", strerror(errno));
		assert(false);
	}

	//check_errno(madvise(addr, total_bytes_count, MADV_SEQUENTIAL));

	return addr;

	#undef MAP_HUGE_2MB
	#undef MAP_HUGE_1GB
}


inline internal void
SYS_RELEASE_PLATFORM_VIRTUAL_MEMORY(void *base_addr, u32 bytes_count) {
	assert(munmap(base_addr, (size_t)bytes_count) == 0);
}