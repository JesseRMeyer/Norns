void _Wait() {
	while (s->lock.exchange(LOCKED) == LOCKED) {
		syscall(SYS_futex, &s->lock, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, LOCKED, 0, 0, 0);
		CPU_PAUSE();
	}
}

void _Wake() {
	assert(s->lock == LOCKED);

	s->lock = UNLOCKED;
	const u32 wake_number_of_waiters = 1;
	syscall(SYS_futex, &s->lock, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, wake_number_of_waiters, 0, 0, 0);
}


void _Sleep() {
	s->lock = WAIT;
	syscall(SYS_futex, &s->lock, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, WAIT, 0, 0, 0);
}

void _Awake() {
	assert(s->lock == WAIT);

	s->lock = ACTIVE;
	const u32 wake_number_of_waiters = 1;
	syscall(SYS_futex, &s->lock, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, wake_number_of_waiters, 0, 0, 0);
}
