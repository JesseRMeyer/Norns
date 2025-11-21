using std::align_val_t;
using std::max_align_t;

#ifndef TSAN

class MemoryManager {
public:
	internal inline MemoryManager&
	Initialize(void) {
		return GetInstance();
	}

	inline tlsf_t&
	GetAllocator() {
		return allocator;
	}

	MemoryManager&
	operator=(MemoryManager& other) = delete;

	MemoryManager&
	operator=(MemoryManager&& other) = delete;

private:
	constexpr inline internal int DEFAULT_ALLOCATION_BYTES_MAX = MEGABYTES(32);
	
	internal inline MemoryManager&
	GetInstance() {
		global MemoryManager instance;
		return instance;
	}

	MemoryManager() {
		void* p = SYS_ACQUIRE_PLATFORM_VIRTUAL_MEMORY(DEFAULT_ALLOCATION_BYTES_MAX);
		ASAN_POISON(p, DEFAULT_ALLOCATION_BYTES_MAX);

		//NOTE(Jesse): tlsf has been configured to be on ASAN's ignore list
		allocator = tlsf_create_with_pool(p, DEFAULT_ALLOCATION_BYTES_MAX);
	}

	MemoryManager(MemoryManager& other) = delete;
	MemoryManager(MemoryManager&& other) = delete;

	~MemoryManager() {
		ASAN_UNPOISON(allocator, DEFAULT_ALLOCATION_BYTES_MAX);

		tlsf_destroy(allocator);

		SYS_RELEASE_PLATFORM_VIRTUAL_MEMORY((void*)allocator, DEFAULT_ALLOCATION_BYTES_MAX);
		
		allocator = nullptr;
	}

	tlsf_t allocator;
};

thread_local auto& __memory_manager = MemoryManager::Initialize();

void *Norns_Realloc(void* p, u32 bytes_count) {
	void* ptr = tlsf_realloc(__memory_manager.GetAllocator(), p, bytes_count);
	ASAN_UNPOISON(ptr, tlsf_block_size(ptr));

	if (p != ptr) { //NOTE(Jesse): If the original allocation could not be resized "inplace", the allocation was moved, so poison the original extent.
		ASAN_POISON(p, tlsf_block_size(p));
	}

	return ptr;
}

void* operator new(size_t bytes_count, align_val_t alignment) {
	void* ptr = tlsf_memalign(__memory_manager.GetAllocator(), (size_t)alignment, bytes_count);
	ASAN_UNPOISON(ptr, tlsf_block_size(ptr));

	assert(ptr);

	return ptr;
}

void* operator new(size_t bytes_count) {
	//NOTE(Jesse): The standard dictates new's default alignment.
	return operator new(bytes_count, align_val_t(alignof(max_align_t)));
}

void* operator new[](size_t bytes_count, align_val_t alignment) {
	void* ptr = tlsf_memalign(__memory_manager.GetAllocator(), (size_t)alignment, bytes_count);
	ASAN_UNPOISON(ptr, tlsf_block_size(ptr));

	assert(ptr);

	return ptr;
}

void operator delete(void* ptr, align_val_t alignment) noexcept {
	(void)alignment;

	tlsf_free(__memory_manager.GetAllocator(), ptr);
	
	ASAN_POISON(ptr, tlsf_block_size(ptr));
}

void operator delete[](void* ptr, align_val_t alignment) noexcept {
	(void)alignment;

	tlsf_free(__memory_manager.GetAllocator(), ptr);
	
	ASAN_POISON(ptr, tlsf_block_size(ptr));
}

void operator delete(void* ptr) noexcept {
	tlsf_free(__memory_manager.GetAllocator(), ptr);

	ASAN_POISON(ptr, tlsf_block_size(ptr));
}

void operator delete(void* ptr, size_t bytes_count) noexcept {
	(void)bytes_count;

	operator delete(ptr);
}

void operator delete(void* ptr, size_t bytes_count, align_val_t alignment) noexcept {
	(void)bytes_count;

	operator delete(ptr, alignment);
}

#endif

#undef ASAN_POISON
#undef ASAN_UNPOISON
