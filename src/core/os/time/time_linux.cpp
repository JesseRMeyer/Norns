void
_Time() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	initial_start_time = SecondsToNano((Second)ts.tv_sec) + (Nano)ts.tv_nsec;
}

inline Nano
_Now() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	Nano ts_combined = SecondsToNano((Second)ts.tv_sec) + (Nano)ts.tv_nsec;

	time_since_construction = ts_combined - initial_start_time;
	return time_since_construction;
}

inline internal void
_Sleep(Second duration) {
	timespec ts = {
		(i32)GetIntegerPartSecond(duration),
		(i32)SecondsToNano(GetFractionalPartSecond(duration)),
	};
	nanosleep(&ts, nil);
}