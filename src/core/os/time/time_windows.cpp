//TODO(Jesse): Start the first sample at half a second before overflow to ensure that case is properly handled.

void
_Time() {
	LARGE_INTEGER qp_starting_time;

	QueryPerformanceFrequency(&qp_frequency);
	QueryPerformanceCounter(&qp_init);

	qp_starting_time.QuadPart = qp_init.QuadPart;

	qp_starting_time.QuadPart *= 1000000000ull; //NOTE(Jesse): To Nanoseconds.
	qp_starting_time.QuadPart /= qp_frequency.QuadPart;

	initial_start_time = qp_starting_time.QuadPart;
}

inline Nano
_Now() {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	now.QuadPart -= qp_init.QuadPart; //NOTE(Jesse): Offset from first sample to keep magnitude relatively small to reduce problems from expansion to nanoseconds next.

	now.QuadPart *= 1000000000ull;
	now.QuadPart /= qp_frequency.QuadPart;

	time_since_construction = now.QuadPart - initial_start_time;

	return time_since_construction;
}

inline internal void
_Sleep(Second duration) {
	::Sleep((DWORD)SecondsToMili(duration)); //NOTE(Jesse): Call Win32's Sleep -- not our wrapper!
}

LARGE_INTEGER qp_frequency = {};
LARGE_INTEGER qp_init = {};