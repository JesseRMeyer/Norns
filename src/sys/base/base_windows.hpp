internal void
PrintLastError() {
	DWORD last_error = GetLastError(); 

	const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS;
	LPSTR buffer = nullptr;
	if (FormatMessage(flags, nullptr, last_error, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		buffer, 0, nullptr) == 0) {
			//TODO(Jesse): Why does the returned buffer return nothing - maybe only on Wine?
			fprintf(stderr, "FormatMessage could not get string for Error Code: %lu\n", last_error);
	} else {
		fprintf(stderr, "Error Code: %lu %s\n", last_error, buffer);
	}

	LocalFree(buffer);
}

inline internal void*
SYS_ACQUIRE_PLATFORM_VIRTUAL_MEMORY(u32 bytes_count) {
	void* ptr = VirtualAlloc(NULL, bytes_count, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (ptr == nullptr) {
		PrintLastError();
	}
	assert(ptr != nullptr and "VirtualAlloc failed!");

	return ptr;
}

inline internal void
SYS_RELEASE_PLATFORM_VIRTUAL_MEMORY(void *base_addr, u32 bytes_count) {
	(void)bytes_count;

	assert(VirtualFree(base_addr, 0, MEM_RELEASE) != 0 and "VirtualFree failed!");
}
