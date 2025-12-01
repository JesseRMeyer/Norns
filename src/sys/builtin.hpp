#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef abs
#undef abs
#endif

template<class T>
const T& min(const T& a, const T& b) {
	return a <= b ? a : b;
}

template<class T>
const T& max(const T& a, const T& b) {
	return a >= b ? a : b;
}

template<class T>
const T& abs(const T& a) {
	return a >= 0 ? a : -a;
}

internal inline
size_t align(uintptr_t p, uintptr_t alignment) {
	return (alignment - (p & (alignment - 1))) & (alignment - 1);
}

template<typename T>
concept TriviallyCopyable = __is_trivially_copyable(T);

template<typename T>
concept Pointer = __is_pointer(T);

template<typename T>
concept Integral = __is_integral(T);

template<typename T>
concept Enum = __is_enum(T);

//NOTE(Jesse): reject void* slices -- use byte or char instead.
template<typename T> requires (not is_void_v<T>)
class Slice {
public:
	Slice(T* ptr, u32 size): data(ptr), size(size) {}
	Slice() = default;

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
		if (data == other.data and size == other.size) {
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
