inline HANDLE 
_Start(void* func, void* data) {
	DWORD ignore;
	thread_id = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)func, data, 0, &ignore);

	return thread_id;
}

inline void 
_Stop() {
	//NOTE(Jesse): The best way to accomplish this is at the application level with some
	// kind of message passing to indicate intent.  This is a last ditch effort.
	if (thread_id != 0) {
		assert(TerminateThread(thread_id, 0) != 0); //TODO(Jesse): GetLastError
	}
	
	thread_id = 0;
}

inline bool 
_IsAlive() {
	if (thread_id == 0) {
		return false;
	}

	return true;
}

HANDLE thread_id = 0;