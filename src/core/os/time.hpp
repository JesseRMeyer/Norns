class Time {
public:
	using Nano = u64;
	using Mili = u64;
	using Second = f32;

	constexpr internal inline Second
	GetIntegerPartSecond(Second seconds) {
		u64 s = seconds;
		return (f64)s;
	}

	constexpr internal inline Second
	GetFractionalPartSecond(Second seconds) {
		u64 s = seconds;
		return (f64)seconds - (f64)s;
	}

	constexpr internal inline Nano
	SecondsToNano(Second seconds) {
		return (f64)1E+09 * (f64)seconds;
	}

	constexpr internal inline Second
	NanoToSeconds(Nano nano) {
		return (f64)nano / (f64)1E+09;
	}

	constexpr internal inline Mili
	SecondsToMili(Second second) {
		return (f64)second * (f64)1E+03;
	}

	constexpr internal inline Mili
	NanoToMili(Nano nano) {
		return (f64)nano / (f64)1E+06 + 0.5;
	}

	internal Nano
	StartTimer() {
		return Now();
	}

	internal Nano
	StopTimer(Nano before) {
		return Duration(before, Now());
	}

	internal Nano
	StopTimer() {
		return Now();
	}

	internal Nano
	Duration(Nano before, Nano after) {
		if (after <= before) {
			return 0;
		}

		return after - before;
	}

	inline internal Nano
	Now() {
		return GetInstance()._Now();
	}

	inline internal Mili
	NowMili() {
		return NanoToMili(Now());
	}

	internal inline void 
	Sleep(Second duration) {
		_Sleep(duration);
	}

	Time(Time& other) = delete;
	Time(Time&& other) = delete;
	Time& operator=(Time& other) = delete;
	Time& operator=(Time&& other) = delete;

	class TimedSection {
	public:
		TimedSection(StringStream& s, Nano now): begin(now), ss(s) {}
		TimedSection(StringStream& s): begin(Now()), ss(s) {}
		~TimedSection() {
			ss << NanoToMili(Time::Now() - begin);
		}

	private:
		Nano begin;
		StringStream& ss;
	};
	
private:
	internal inline Time&
	GetInstance() {
		global Time instance;
		return instance;
	}

	Time(): initial_start_time(0), time_since_construction(0) {
		_Time();
	}

	#include PLATFORM_CPP(time/time)

	atomic<Nano> initial_start_time;
	atomic<Nano> time_since_construction;
};

auto __time_ignored = Time::Now(); //NOTE(Jesse): Instantiate at launch

