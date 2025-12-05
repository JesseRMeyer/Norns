struct GridCost {
	u16 v;

	constexpr bool
	operator>=(const GridCost& other) const {
		return not (v < other.v);
	};

	constexpr bool
	operator<=(const GridCost& other) const {
		return not (v > other.v);
	};

	constexpr GridCost
	operator+(const GridCost& other) const {
		return {v + other.v};
	}

	constexpr bool
	operator==(const GridCost& other) const {
		return v == other.v;
	}

	constexpr bool
	operator>(const GridCost& other) const {
		return v > other.v;
	}

	constexpr bool
	operator<(const GridCost& other) const {
		return v < other.v;
	}
};

struct GridCell {
	u16 x;
	u16 y;

	constexpr bool
	operator==(const GridCell& other) const {
		return (x == other.x) and (y == other.y);
	};

	constexpr GridCost
	ManhattanDistanceHeuristic(const GridCell& other) {
		return {abs((i32)x - (i32)other.x) + abs((i32)y - (i32)other.y)};
	}

	friend StringStream&
	operator<<(StringStream& s, const GridCell& cell) {
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
		#pragma clang diagnostic ignored "-Wformat-extra-args"

		char buffer[32] = {};
		int characters_written = stbsp_snprintf((char*)&buffer[0], size(buffer), "y: %_$$$d, x: %_$$$d", cell.y, cell.x);
		(void)characters_written;
		s << (char*)buffer;

		#pragma clang diagnostic pop

		return s;
	}
};

template <typename T>
struct Grid2D { //NOTE(Jesse): 0, 0 is top left, growing down right
	T* data; //NOTE(Jesse): Grid doesn't manage the grid's memory!
	u16 x;
	u16 y;

	constexpr internal inline u16 INVALID_CELL_IDX = -1;
	constexpr internal inline GridCell INVALID_CELL = {INVALID_CELL_IDX, INVALID_CELL_IDX};

	GridCost
	NeighborCost(GridCell& start, GridCell& neighbor) {
		assert(start.ManhattanDistanceHeuristic(neighbor).v == 1);

		T start_value = data[Index(start)];
		T neighbor_value = data[Index(neighbor)];
		T cost = (i32)neighbor_value - (i32)start_value;
		return {cost};
	}

	template <typename U>
	constexpr inline bool
	IsValid(U&& cell) {
		if (cell.y >= y or cell.x >= x) {
			return false;
		}

		if (cell.y == INVALID_CELL_IDX or cell.x == INVALID_CELL_IDX) {
			return false;
		}

		return true;
	}

	struct Neighbors {
		GridCell N[4];

		auto begin() {
			return &N[0];
		}
		
		auto end() {
			return &N[0] + size(N);
		}
	};

	template <typename U>
	inline T&
	operator[](U&& cell) {
		assert(IsValid(cell));

		return data[Index(forward<U>(cell))];
	}

	template <typename U>
	inline u32
	Index(U&& cell) {
		return clamp(cell.y, u16(0), u16(y - 1)) * x + clamp(cell.x, u16(0), u16(x - 1));
	}

	inline GridCell
	GetNorthNeighbor(GridCell& cell) {
		return {cell.x < x ? cell.x : INVALID_CELL_IDX, cell.y > 0 ? cell.y - 1 : INVALID_CELL_IDX};
	}

	inline GridCell
	GetEastNeighbor(GridCell& cell) {
		return {cell.x < (x - 1) ? cell.x + 1 : INVALID_CELL_IDX, cell.y < y ? cell.y: INVALID_CELL_IDX};
	}

	inline GridCell
	GetSouthNeighbor(GridCell& cell) {
		return {cell.x < x ? cell.x : INVALID_CELL_IDX, cell.y < (y - 1) ? cell.y + 1 : INVALID_CELL_IDX};
	}

	inline GridCell
	GetWestNeighbor(GridCell& cell) {
		return {cell.x > 0 ? cell.x - 1 : INVALID_CELL_IDX, cell.y < y ? cell.y : INVALID_CELL_IDX};
	}

	inline Neighbors
	GetNeighbors(GridCell& cell) {
		return {GetNorthNeighbor(cell), GetEastNeighbor(cell), GetSouthNeighbor(cell), GetWestNeighbor(cell)};
	}
};

template <typename T>
Vector<GridCell> 
AStar(Grid2D<T>& graph, GridCell& start, GridCell& goal) {
	auto frontier = PriorityQueue<GridCell, GridCost>();
	auto came_from = HashTable<GridCell, GridCell>();
	auto cost_from = HashTable<GridCell, GridCost>();

	frontier.Put(start, {0});
	came_from[start] = Grid2D<T>::INVALID_CELL;
	cost_from[start] = {0};

	while (not frontier.IsEmpty()) {
		auto current = frontier.PopTop();
		if (current == goal) {
			break;
		}

		for (auto& next: graph.GetNeighbors(current)) {
			if (not graph.IsValid(next)) {
				continue;
			}

			GridCost new_cost = cost_from[current] + graph.NeighborCost(current, next);
			if (auto next_in = cost_from.Find(next); cost_from.Found(next_in) and new_cost >= next_in->v) {
				continue;
			}
			
			frontier.Put(next, new_cost + goal.ManhattanDistanceHeuristic(next));

			cost_from[next] = new_cost;
			came_from[next] = current;
		}
	}

	auto path = Vector<GridCell>(came_from.Size());
	auto current = goal;
	bool complete = true;
	while (current != start and graph.IsValid(current)) { 
		path.PushBack(current);
		
		auto next = came_from.Find(current);
		if (not came_from.Found(next)) {
			complete = false;
			break;
		}

		current = next->v;
	}

	//assert(current == start);
	
	if (complete) {
		path.PushBack(start);
		path.Reverse();
	}

	return path;
}
