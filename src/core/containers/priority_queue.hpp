template <typename T, Arithmetic P, template<typename> typename Comp=less> //NOTE(Jesse): Defaults to max heap
class PriorityQueue: public Heap<T, P, Comp> {
	using Base = Heap<T, P, Comp>;
	
public:
	PriorityQueue() = default;
	PriorityQueue(int capacity): Base(capacity) {}
	PriorityQueue(PriorityQueue& other): Base(other) {}
	PriorityQueue(PriorityQueue&& other) = default;
	~PriorityQueue() = default;

	PriorityQueue& 
	operator=(PriorityQueue& other) = default;

	PriorityQueue& 
	operator=(PriorityQueue&& other) = default;

	template <typename U>
	void
	Put(U&& t, P p) {
		//NOTE(Jesse): Pretty wild how many different language ideas have to compose to achieve this.
		Base::items.PushBack(typename Base::pair{forward<U>(t), p});
		Base::HeapifyUp(Base::items.Size() - 1);
	}
	
	inline T& 
	Top() { 
		return Base::items[0].t;
	}

	inline P&
	TopPriority() {
		return Base::items[0].p;
	}
	
	void 
	Pop() {
		if (Base::items.Empty()) {
			return;
		}

		Base::items[0] = Base::items.Back();
		Base::items.PopBack();
		if (not Base::items.Empty()) { 
			Base::HeapifyDown(0);
		}
	}

	T 
	PopTop() {
		assert(not Base::items.Empty());

		T result = Base::items[0].t;

		Base::items[0] = Base::items.Back();
		Base::items.PopBack();
		if (not Base::items.Empty()) { 
			Base::HeapifyDown(0);
		}

		return result;
	}

	bool
	IsEmpty() {
		return Base::items.Size() == 0;
	}
};