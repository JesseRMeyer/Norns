template <typename T>
class Vector {
public:
	Vector(): data(nullptr), size(0), capacity(0) {}

	Vector(u32 initial_capacity): data(new T[initial_capacity]{}), size(0), capacity(initial_capacity) {}
	
	~Vector() {
		Cleanup();
	}

	Vector(Vector& other) = delete;

	Vector(Vector&& other) {
		data = other.data;
		size = other.size;
		capacity = other.capacity;

		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;
	}

	auto begin() const {
		return &data[0];
	}

	auto end() const {
		return &data[size];
	}

	Vector& 
	operator=(Vector& other) = delete;

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

	inline u32
	Capacity() {
		return capacity;
	}

	inline T&
	Emplace(T&& t) {
		if (not HasRoom()) {
			capacity *= 2u;
			data = (T*)Norns_Realloc(data, capacity * sizeof(T));
		}

		data[size++] = move(t);
		return data[size - 1];
	}

	T& 
	operator[](u32 idx) {
		assert(idx < capacity);

		return data[idx];
	}

	u32 
	Size() {
		return size;
	}

private:
	void
	Cleanup() {
		if (data != nullptr) {
			delete[] data;
		}

		data = nullptr;
		size = 0;
		capacity = 0;
	}

	T* data = nullptr;
	u32 size = 0;
	u32 capacity = 0;
};