template <typename T>
class Vector {
public:
	Vector() = default;
	Vector(u32 initial_capacity): data(new T[initial_capacity]{}), size(0), capacity(initial_capacity) {}
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

	auto begin() const {
		return &data[0];
	}

	auto end() const {
		return &data[size];
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

	inline bool
	HasRoom() {
		return size < capacity;
	}

	inline bool
	Empty() {
		return not HasRoom();
	}

	inline u32
	Capacity() {
		return capacity;
	}

	template <typename U>
	inline T&
	PushBack(U&& t) {
		if (not HasRoom()) {
			capacity *= 2u;
			data = (T*)Norns_Realloc(data, capacity * sizeof(T));
		}

		data[size++] = forward<U>(t);
		return data[size - 1];
	}

	T& 
	operator[](u32 idx) {
		assert(idx < capacity);

		return data[idx];
	}

	T 
	PopBack() {
		assert(size > 0);

		return data[--size];
	}

	u32 
	Size() {
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