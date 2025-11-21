template <typename T>
class SharedPtr {
public:
	SharedPtr(): ptr(new T{}), ref_count(new atomic<i32>(1)) {}
	SharedPtr(T* p): ptr(p), ref_count(new atomic<i32>(1)) {}

	SharedPtr(T& t): ptr(new T(t)), ref_count(new atomic<i32>(1)) {}
	SharedPtr(T&& t): ptr(new T(forward<T>(t))), ref_count(new atomic<i32>(1)) {}
	SharedPtr(SharedPtr&& other): ptr(other.ptr), ref_count(other.ref_count) {
		other.ptr = nullptr;
		other.ref_count = nullptr;
	}

	SharedPtr(SharedPtr& other): ptr(other.ptr), ref_count(other.ref_count) {
		if (ref_count != nullptr) {
			++(*ref_count);
		}
	}

	SharedPtr& 
	operator=(SharedPtr&& other) {
		if (this == &other) {
			return *this;
		}

		Release();

		ptr = other.ptr;
		ref_count = other.ref_count;

		other.ptr = nullptr;
		other.ref_count = nullptr;

		return *this;
	}

	SharedPtr& 
	operator=(SharedPtr& other) {
		if (this == &other) {
			return *this;
		}

		Release();

		ptr = other.ptr;
		ref_count = other.ref_count;
		++(*ref_count);

		return *this;
	}

	~SharedPtr() {
		Release();
	}

	inline T* 
	operator->() {
		return ptr;
	}

	inline const T* 
	operator->() const {
		return ptr;
	}

	inline T& 
	operator*(){
		return *ptr;
	}

	inline const T& 
	operator*() const {
		return *ptr;
	}

	inline i32
	RefCount() {
		return *ref_count;
	}

private:
	void inline
	Release() {
		if (ref_count != nullptr and (--(*ref_count)) == 0) {
			delete ptr;
			delete ref_count;

			ptr = nullptr;
			ref_count = nullptr;
		}
	}

	T* ptr = nullptr;
	atomic<i32>* ref_count = nullptr;
};