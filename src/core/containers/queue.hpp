//TODO(Jesse): Provide a queue that dynamically grows.  But this can hide misuse bugs.

template <typename T>
class AtomicFixedSizeQueue { //TODO(Jesse): Optimal acquire/release memory ordering (right now the result from every atomic operation is the most conservative via seq_cst)
public:
	AtomicFixedSizeQueue(u32 max_elements): data(new atomic<T>[max(u32(2), max_elements)]{}), capacity(max(u32(2), max_elements)) {}

	AtomicFixedSizeQueue(AtomicFixedSizeQueue&& other) {
		if (this == &other) {
			//NOTE(Jesse): Bug?
			return;
		}

		Cleanup();

		data = other.data;
		head_idx.store(other.head_idx.load());
		tail_idx.store(other.tail_idx.load());
		capacity.store(other.capacity.load());

		other.data = nullptr;
		other.head_idx = 0;
		other.tail_idx = 0;
		other.capacity = 0;
	}

	AtomicFixedSizeQueue& 
	operator=(AtomicFixedSizeQueue&& other) {
		if (this == &other) {
			return *this;
		}

		Cleanup();

		data = other.data;
		head_idx.store(other.head_idx.load());
		tail_idx.store(other.tail_idx.load());
		capacity.store(other.capacity.load());

		other.data = nullptr;
		other.head_idx = 0;
		other.tail_idx = 0;
		other.capacity = 0;

		return *this;
	}

	AtomicFixedSizeQueue(AtomicFixedSizeQueue const& other) = delete;
	
	AtomicFixedSizeQueue& 
	operator=(AtomicFixedSizeQueue const& other) = delete;

	~AtomicFixedSizeQueue() {
		Cleanup();
	}

	template <typename U>
	inline void
	Put(U&& item) {
		assert(HasRoom());

		defer(head_idx = (head_idx + 1) % capacity);

		data[head_idx] = forward<U>(item);
	}

	inline T&
	Front() {
		assert(not IsEmpty());

		return data[tail_idx];
	}

	//NOTE(Jesse): Return by value which is not abi compatible with STD due to concerns around exceptions (which this project disables)
	// and a minor point around copy by value inefficiencies, but the types provided to this queue usually
	// define move copy constructors by default and thus shouldn't be problematic (copy elided).
	// Otherwise, hopefully this call is inlined and dead code elimination removes the dead write.
	//
	// Optionally, the return value can be ignored and Front() used instead.
	inline T
	Pop() { 
		assert(not IsEmpty());

		defer(tail_idx = (tail_idx + 1) % capacity);
		
		return move(data[tail_idx]); 
	}

	inline bool 
	IsEmpty() {
		return head_idx == tail_idx;
	}

	inline bool 
	IsFull() {
		return not HasRoom();
	}

	inline bool 
	HasRoom() {
		return (head_idx + 1) % capacity != tail_idx;
	}

	inline bool 
	HasItems() {
		return not IsEmpty();
	}

protected:
	void 
	Cleanup() {
		delete[] data;
		data = nullptr;

		head_idx = 0;
		tail_idx = 0;
		capacity = 0;
	}

	atomic<T> *data = nullptr;
	atomic<u32> head_idx = 0;
	atomic<u32> tail_idx = 0;
	atomic<u32> capacity = 0;
};

template <typename T, typename MutexType>
class ThreadSafeFixedSizeQueue: public AtomicFixedSizeQueue<T> {
	using Base = AtomicFixedSizeQueue<T>;
public:
	ThreadSafeFixedSizeQueue(u32 capacity): Base(capacity), mutex() {}
	ThreadSafeFixedSizeQueue(ThreadSafeFixedSizeQueue& other) = delete;
	ThreadSafeFixedSizeQueue(ThreadSafeFixedSizeQueue&& other): Base(move(other)) {}

	ThreadSafeFixedSizeQueue&
	operator=(ThreadSafeFixedSizeQueue& other) = delete;

	ThreadSafeFixedSizeQueue&
	operator=(ThreadSafeFixedSizeQueue&& other) = default;

	inline T
	Pop() {
		mutex.Lock();
		defer(mutex.Unlock());

		return Base::Pop();
	}

	template <typename U>
	inline void
	Put(U&& item) {
		mutex.Lock();
		defer(mutex.Unlock());

		Base::Put(forward<U>(item));
	}

private:
	MutexType mutex;
};