#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef abs
#undef abs
#endif

#define NOT_IMPLEMENTED()

#ifdef __arm__
    #define CPU_PAUSE() __asm__ volatile("yield" ::: "memory")
#elif defined(__x86_64__)
	#define CPU_PAUSE() __builtin_ia32_pause()
#else
	#define CPU_PAUSE() NOT_IMPLEMENTED()
#endif

//NOTE(Jesse): Unintuitive to see these as structs
// but this allows for generic types like
// PriorityQueue to template over the provided
// operator since they ARE types - not functions!
template<class T>
struct less {
	constexpr bool 
	operator()(const T& a, const T& b) const {
		return a < b;
	}
};

template<class T>
struct greater {
	constexpr bool 
	operator()(const T& a, const T& b) const {
		return a > b;
	}
};

template<class T>
internal const T& 
min(const T& a, const T& b) {
	return a <= b ? a : b;
}

template<class T>
internal const T& 
max(const T& a, const T& b) {
	return a >= b ? a : b;
}

template<class T>
internal const T& 
abs(const T& a) {
	return a >= 0 ? a : -a;
}

template<class T>
internal const T& 
saturate(const T& a) {
	return a > 1 ? 1 : a < 0 ? 0 : a;
}

template<class T>
internal const T& 
clamp(const T& a, const T& lower, const T& upper) {
	return a > upper ? upper : a < lower ? lower : a;
}

internal inline size_t 
align(uintptr_t p, uintptr_t alignment) {
	return (alignment - (p & (alignment - 1))) & (alignment - 1);
}

template <class T, size_t N>
internal constexpr inline size_t 
size(const T (&)[N]) noexcept {
	return N;
}

template<typename T>
concept TriviallyCopyable = __is_trivially_copyable(remove_cvref_t<T>);

template<typename T>
concept Pointer = __is_pointer(T);

template<typename T>
concept Integral = __is_integral(remove_cvref_t<T>);

template<typename T>
concept Real = __is_floating_point(remove_cvref_t<T>);

template<typename T>
concept BaseArithmetic = Integral<T> or Real<T>;

template<typename T>
concept Arithmetic = BaseArithmetic<T> or
    requires(T t) {
        { t.v } -> BaseArithmetic;
    };

template<typename T>
concept Enum = __is_enum(T);

//NOTE(Jesse): reject void* slices -- use byte or char instead.
template<typename T> requires (not is_void_v<T>)
class Slice {
public:
	Slice() = default;
	Slice(T* ptr, u32 size): data(ptr), size(size) {}

	template <u32 N>
	Slice(T (&array)[N]): Slice(array, N) {}

	Slice(Slice& other): data(other.data), size(other.size) {}
	Slice(Slice&& other): data(other.data), size(other.size) {
		other.data = nullptr;
		other.size = 0;
	}

	~Slice() {
		data = nullptr;
		size = 0;
	}

	Slice&
	operator=(Slice& other) {
		if (this == &other) {
			return *this;
		}

		data = other.data;
		size = other.size;

		return *this;
	}

	Slice&
	operator=(Slice&& other) {
		if (this == &other) {
			return *this;
		}

		data = other.data;
		size = other.size;

		other.data = nullptr;
		other.size = 0;

		return *this;
	}

	inline T&
	operator[](u32 idx) {
		assert(idx < size);
		
		return data[idx];
	}

	inline T* 
	Data() {
		return data;
	}

	inline u32
	Size() {
		return size;
	}

	auto begin() const {
		return &data[0];
	}

	auto end() const {
		return &data[size];
	}

	bool
	operator==(Slice& other) {
		if (this == &other) {
			return true;
		}

		if (size != other.size) {
			return false;
		}

		for (u32 idx = 0; idx < size; ++idx) {
			if (data[idx] != other[idx]) {
				return false;
			}
		}

		return true;
	}

private:
	T* data = nullptr;
	u32 size = 0;
};

template<typename T>
concept Hashable = TriviallyCopyable<T>;

//NOTE(Jesse): Sliceable types have first class hashing support.
template<typename T>
concept IsSliceCompatible =
	requires(T& v) {
		{ v.Data() } -> Pointer;
		{ v.Size() } -> Integral;
	};

#include "../core/misc/pcg.hpp"

//NOTE(Jesse): Universally fail except for specific cases.
template<typename T>
struct Hash;

template<Hashable T>
struct Hash<T> {
	template <typename U>
	constexpr u32 operator()(U&& v) {
		return pcg32_hash((byte*)&v, sizeof(T));
	}

	constexpr u32 operator()(T* v) {
		return pcg32_hash((byte*)v, sizeof(T));
	}
};

template<IsSliceCompatible T>
struct Hash<T> {
	constexpr u32 operator()(T& v) {
		using base_type = remove_pointer_t<decltype(v.Data())>;
		return pcg32_hash((byte*)v.Data(), v.Size() * sizeof(base_type));
	}
};

template<Enum T>
struct Hash<T> {
	constexpr u32 operator()(T v) {
		using U = __underlying_type(T); //NOTE(Jesse): Underlying is integer.
		return Hash<U>{}((U)v);
	}
};
