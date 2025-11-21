template <typename T>
class UniquePtr {
public:
	UniquePtr(): ptr(new T{}) {}

	UniquePtr(T& t): ptr(new T(t)) {}
	UniquePtr(T&& t): ptr(new T(forward<T>(t))) {}
	UniquePtr(UniquePtr&& other): ptr(other.ptr) {
		other.ptr = nullptr;
	}

	explicit UniquePtr(T* p): ptr(p) {p = nil;}

	UniquePtr(UniquePtr& other) = delete;

	UniquePtr& 
	operator=(UniquePtr& other) = delete;

	UniquePtr& 
	operator=(UniquePtr&& other) {
		if (this == &other) {
			return *this;
		}

		Release();

		ptr = other.ptr;
		other.ptr = nullptr;

		return *this;
	}

	~UniquePtr() {
		Release();
	}

	UniquePtr& 
	operator=(nullptr_t) {
    	Release();
    	return *this;
	}

	inline T* 
	operator->() {
		return ptr;
	}

	inline T& 
	operator*(){
		return *ptr;
	}

	inline const T* 
	operator->() const {
		return ptr;
	}

	inline const T& 
	operator*() const {
		return *ptr;
	}

private:
	void inline
	Release() {
		delete ptr;	
		ptr = nullptr;
	}

	T* ptr = nullptr;
};
