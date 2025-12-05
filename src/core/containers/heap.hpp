template <typename T, Arithmetic P, template<typename> typename Comp=less> //NOTE(Jesse): Defaults to max heap
class Heap {
public:
	Heap(): items(32) {}
	Heap(u16 capacity): items(capacity) {}

	u16 inline
	ParentIdx(u16 child_idx) {
		return (child_idx - 1) / 2;
	}
	
	u16 inline
	LeftChildIdx(u16 parent_idx) {
		return (2 * parent_idx) + 1;
	}

	u16 inline
	RightChildIdx(u16 parent_idx) {
		return (2 * parent_idx) + 2;
	}

	void 
	HeapifyDown(u16 node_idx) {
		u16 heap_size = items.Size();
		while (true) {
			u16 chosen_idx = node_idx;
			u16 left_idx = LeftChildIdx(node_idx);
			u16 right_idx = RightChildIdx(node_idx);
			
			if (left_idx < heap_size && Compare(items[chosen_idx].p, items[left_idx].p)) {
				chosen_idx = left_idx;
			}
			
			if (right_idx < heap_size && Compare(items[chosen_idx].p, items[right_idx].p)) {
				chosen_idx = right_idx;
			}
			
			if (chosen_idx == node_idx) {
				break;
			}

			swap(items[node_idx], items[chosen_idx]);
			node_idx = chosen_idx;
		}
	}

	void 
	HeapifyUp(u16 idx) {
		while (idx > 0) {
			u16 parent_idx = ParentIdx(idx);
			if (not Compare(items[parent_idx].p, items[idx].p)) {
				break;
			}

			swap(items[parent_idx], items[idx]);
			idx = parent_idx;
		}
	}

protected:
	struct pair {
		T t;
		P p;
	};

	Vector<pair> items;

private:
	Comp<P> Compare;
};