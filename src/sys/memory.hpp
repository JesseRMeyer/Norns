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
		thread_local MemoryManager instance;
		return instance;
	}

	MemoryManager() {
		void* p = SYS_ACQUIRE_PLATFORM_VIRTUAL_MEMORY(DEFAULT_ALLOCATION_BYTES_MAX);
		ASAN_POISON(p, DEFAULT_ALLOCATION_BYTES_MAX);

		//NOTE(Jesse): tlsf has been configured to be on ASAN's ignore list
		allocator = tlsf_create_with_pool(p, DEFAULT_ALLOCATION_BYTES_MAX);
	}

	~MemoryManager() {
		ASAN_UNPOISON(allocator, DEFAULT_ALLOCATION_BYTES_MAX);

		assert(tlsf_check(allocator) == 0);

		tlsf_destroy(allocator);

		SYS_RELEASE_PLATFORM_VIRTUAL_MEMORY((void*)allocator, DEFAULT_ALLOCATION_BYTES_MAX);
		
		allocator = nullptr;
	}

	MemoryManager(MemoryManager& other) = delete;
	MemoryManager(MemoryManager&& other) = delete;

	tlsf_t allocator;
};

//NOTE(Jesse): Apparently thread_local variable destructors occur
// _before_ "standard" global variable destructors.  This is kind of bad here
// when objects being destroyed assume their memory is still valid, but here,
// since each thread has its own MemoryManager, the manager is destroyed
// before their thread's global variables.  In practice this is harmless,
// because the memory would be made invalid / freed anyway, but, this causes
// a segfault during process exit if, for example, there is a shared StringStream
// across the main thread and aux threads.
thread_local auto& __memory_manager = MemoryManager::Initialize();

void *
Norns_Realloc(void* p, u32 bytes_count) {
	if (p == nullptr) {
		return nullptr;
	}

	void* ptr = tlsf_realloc(__memory_manager.GetAllocator(), p, bytes_count);
	ASAN_UNPOISON(ptr, tlsf_block_size(ptr));

	assert(ptr != nullptr);

	if (p != ptr) { //NOTE(Jesse): If the original allocation could not be resized "inplace", the allocation was moved, so poison the original extent.
		ASAN_POISON(p, tlsf_block_size(p));
	}

	return ptr;
}

void* 
operator new(size_t bytes_count, align_val_t alignment) {
	void* ptr = tlsf_memalign(__memory_manager.GetAllocator(), (size_t)alignment, bytes_count);
	ASAN_UNPOISON(ptr, tlsf_block_size(ptr));

	assert(ptr != nullptr);

	return ptr;
}

void 
operator delete(void* ptr) noexcept {
	if (ptr == nullptr) {
		return;
	}

	tlsf_free(__memory_manager.GetAllocator(), ptr);

	ASAN_POISON(ptr, tlsf_block_size(ptr));
}

void* 
operator new(size_t bytes_count) {
	//NOTE(Jesse): The standard dictates new's default alignment.
	return operator new(bytes_count, align_val_t(alignof(max_align_t)));
}

void* 
operator new[](size_t bytes_count, align_val_t alignment) {
	void* ptr = tlsf_memalign(__memory_manager.GetAllocator(), (size_t)alignment, bytes_count);
	ASAN_UNPOISON(ptr, tlsf_block_size(ptr));

	assert(ptr != nullptr);

	return ptr;
}

void* 
operator new[](size_t bytes_count) {
	return operator new[](bytes_count, align_val_t(alignof(max_align_t)));
}

void 
operator delete(void* ptr, align_val_t alignment) noexcept {
	(void)alignment;

	operator delete(ptr);
}

void 
operator delete[](void* ptr, align_val_t alignment) noexcept {
	(void)alignment;

	operator delete(ptr);
}

void 
operator delete[](void* ptr) noexcept {
	operator delete(ptr);
}

void 
operator delete[](void* ptr, size_t bytes_count) noexcept {
	(void)bytes_count;
	operator delete(ptr);
}

void 
operator delete(void* ptr, size_t bytes_count) noexcept {
	(void)bytes_count;

	operator delete(ptr);
}

void 
operator delete(void* ptr, size_t bytes_count, align_val_t alignment) noexcept {
	(void)bytes_count;

	operator delete(ptr, alignment);
}

#endif

#undef ASAN_POISON
#undef ASAN_UNPOISON
