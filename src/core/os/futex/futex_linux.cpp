void _Wait() {
	while (s->lock.exchange(LOCKED) == LOCKED) {
		syscall(SYS_futex, &s->lock, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, LOCKED, 0, 0, 0);
		CPU_PAUSE();
	}
}

void _Wake() {
	assert(s->lock.load() == LOCKED);

	s->lock = UNLOCKED;
	const u32 wake_number_of_waiters = 1;
	syscall(SYS_futex, &s->lock, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, wake_number_of_waiters, 0, 0, 0);
}
