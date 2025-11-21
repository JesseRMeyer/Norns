//NOTE(Jesse): These threads are never detached.

class Thread {
public:
	using ThreadID = void*;

	Thread(): f(nullptr), d(nullptr) {};
	Thread(void* func, void* data): f(func), d(data){
		Start();
	};

	inline ThreadID 
	Start(void* func, void* data) {
		assert(func != nullptr and f == nullptr and d == nullptr);
		assert(not IsAlive());
		
		return (ThreadID)_Start(func, data);
	}

	inline ThreadID 
	Start() {
		assert(f != nullptr);
		assert(not IsAlive());
		
		return (ThreadID)_Start(f, d);
	}

	inline void 
	Stop() {
		_Stop();
	}

	inline 
	~Thread() {
		Stop();
	}

	inline bool 
	IsAlive() {
		return _IsAlive();
	}

	Thread(Thread& other) = delete;
	Thread(Thread&& other) = delete;
	Thread& operator=(Thread& other) = delete;
	Thread& operator=(Thread&& other) = delete;

private:
	void *f = nullptr;
	void *d = nullptr;

	#include PLATFORM_CPP(thread/thread)
};