class VirtualMemory {
public:
	inline internal void*
	AcquireVirtualMemory(u32 bytes_count) {
		return _AcquireVirtualMemory(bytes_count);
	}

	inline internal void
	ReleaseVirtualMemory(void* p, u32 bytes_count) {
		_ReleaseVirtualMemory(p, bytes_count);
	}

private:
	#include PLATFORM_CPP(virtual_memory/virtual_memory)
};