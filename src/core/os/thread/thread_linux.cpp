inline pthread_t 
_Start(void* func, void* data) {
	assert(pthread_create(&thread_id, nil, (void *(*)(void *))func, data) == 0);

	return thread_id;
}

inline void 
_Stop() {
	if (thread_id != 0) {
		pthread_join(thread_id, nil);
	}
	
	thread_id = 0;
}

inline bool 
_IsAlive() {
	if (thread_id == 0) {
		return false;
	}

	return pthread_kill(thread_id, 0) != ESRCH;
}

pthread_t thread_id = 0;