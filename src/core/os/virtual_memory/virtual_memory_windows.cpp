inline internal void*
_AcquireVirtualMemory(u32 bytes_count) {
	void* ptr = VirtualAlloc(nullptr, bytes_count, MEM_COMMIT | MEM_RESERVE, {});
	assert(ptr != nullptr and "VirtualAlloc failed!");
	//TODO(Jesse): FormatMessageA

	return ptr;
}

inline internal void
_ReleaseVirtualMemory(void *base_addr, u32 bytes_count) {
	(void)bytes_count;

	assert(VirtualFree(base_addr, 0, MEM_RELEASE) != 0 and "VirtualFree failed!");
}