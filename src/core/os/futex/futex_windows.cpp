void _Wait() {
	while (s->lock.exchange(LOCKED) == LOCKED) {
		assert(WaitOnAddress(&s->lock, &s->lock, sizeof(&s->lock), INFINITE) == TRUE);
	}
}

void _Wake() {
	assert(s->lock.load() == LOCKED);

	s->lock = UNLOCKED;
	WakeByAddressSingle(&s->lock);
}
