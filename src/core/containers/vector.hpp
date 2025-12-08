template <typename T>
class Vector {
public:
	Vector(): data(new T[16]), size(0), capacity(16) {}
	Vector(u32 initial_capacity): data(new T[initial_capacity]{}), size(0), capacity(initial_capacity) {}
	Vector(u32 initial_size, u32 initial_capacity): data(new T[initial_capacity]{}), size(initial_size), capacity(initial_capacity) {
		assert(initial_size <= initial_capacity);
	}
	Vector(T* d, u32 size, u32 capacity): data(d), size(size), capacity(capacity){}
	Vector(Vector const& other) = delete;
	Vector(Vector&& other) {
		data = other.data;
		size = other.size;
		capacity = other.capacity;

		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;
	}
	~Vector() {
		Cleanup();
	}

	global
	Vector<T>
	Null() {
		return Vector<T>(nullptr, 0, 0);
	}

	auto begin() const {
		return &data[0];
	}

	auto end() const {
		return &data[0] + size;
	}

	Vector& 
	operator=(Vector const& other) = delete;

	Vector& 
	operator=(Vector&& other) {
		if (this == &other) {
			return *this;
		}

		Cleanup();

		data = other.data;
		size = other.size;
		capacity = other.capacity;

		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;

		return *this;
	}

	void
	Reserve(u32 new_capacity) {
		if (new_capacity <= capacity) {
			return;
		}

		capacity = new_capacity;
		data = (T*)Norns_Realloc(data, capacity * sizeof(T));
	}

	void
	Reverse() {
		for (u32 idx = 0; idx < size / 2; ++idx) {
			swap(data[idx], data[size - 1 - idx]);
		}
	}

	inline bool
	HasRoom() {
		return size < capacity;
	}

	inline bool
	Empty() {
		return size == 0;
	}

	inline u32
	Capacity() {
		return capacity;
	}

	template <typename U>
	inline T&
	PushBack(U&& t) {
		if (not HasRoom()) {
			Grow();
		}

		data[size++] = forward<U>(t);
		return data[size - 1];
	}

	template<typename... Args> //NOTE(Jesse): Disgusting.
	inline T&
	EmplaceBack(Args&&... args) {
		if (not HasRoom()) {
			Grow();
		}

		//new (&data[0] + size) T(forward<Args>(args)...);
		return data[size++] = move(T(forward<Args>(args)...));
	}

	void inline
	Grow(u8 factor = 2) {
		capacity *= factor;
		capacity = max(u32(8), capacity);

		if (data == nullptr) {
			data = new T[capacity];
			return;
		}

		data = (T*)Norns_Realloc(data, capacity * sizeof(T));
	}

	T& 
	operator[](u32 idx) {
		assert(idx < size);

		return data[idx];
	}

	T const& 
	operator[](u32 idx) const {
		assert(idx < size);

		return data[idx];
	}

	T 
	PopBack() {
		assert(size > 0);

		return data[--size];
	}

	u32 
	Size() const {
		return size;
	}

	T&
	Back() {
		return data[size - 1];
	}

	T&
	Front() {
		assert(size > 0);
		
		return data[0];
	}

private:
	void
	Cleanup() {
		delete[] data;

		data = nullptr;
		size = 0;
		capacity = 0;
	}

	T* data = nullptr;
	u32 size = 0;
	u32 capacity = 0;
};