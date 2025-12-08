struct GridCost {
	i16 v;

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

	constexpr inline bool
	operator==(const GridCell& other) const {
		return (x == other.x) and (y == other.y);
	};

	constexpr inline GridCost
	ManhattanDistanceHeuristic(const GridCell& other) {
		return {abs((i32)x - (i32)other.x) + abs((i32)y - (i32)other.y)};
	}

	constexpr inline GridCost
	EuclidDistanceHeuristic(const GridCell& other) {
		i32 _x = (i32)other.x - (i32)x;
		i32 _y = (i32)other.y - (i32)y;
		return {sqrt(f32(_x * _x) + f32(_y * _y)) + 0.5f};
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
class Grid2D { //NOTE(Jesse): 0, 0 is top left, growing down right
public:
	Grid2D(T* data, u16 x, u16 y): data(data), x(x), y(y) {}

	u16 GetX() {
		return x;
	}

	u16 GetY() {
		return y;
	}

	constexpr internal inline u16 INVALID_CELL_IDX = -1;
	constexpr internal inline GridCell INVALID_CELL = {INVALID_CELL_IDX, INVALID_CELL_IDX};

	GridCost inline 
	NeighborCost(GridCell& start, GridCell& neighbor) {
		assert(start.ManhattanDistanceHeuristic(neighbor).v == 1);

		T& start_value = data[Index(start)];
		T& neighbor_value = data[Index(neighbor)];

		return {(i32)neighbor_value - (i32)start_value};
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

private:
	T* data; //NOTE(Jesse): Grid doesn't manage the grid's memory!
	
	u16 x;
	u16 y;
};

template <typename T>
Vector<GridCell> 
AStar(Grid2D<T>& graph, GridCell& start, GridCell& goal) {
	auto frontier = PriorityQueue<GridCell, GridCost, greater>();
	auto came_from = HashTable<GridCell, GridCell>();
	auto cost = HashTable<GridCell, GridCost>();

	frontier.Put(start, {0});
	came_from[start] = Grid2D<T>::INVALID_CELL;
	cost[start] = {0};

	bool complete = false;
	while (not frontier.IsEmpty()) {
		auto current = frontier.PopTop();
		if (current == goal) {
			complete = true;
			break;
		}

		GridCost current_cost = cost[current];
		for (auto& neighbor: graph.GetNeighbors(current)) {
			if (not graph.IsValid(neighbor)) {
				continue;
			}

			GridCost new_cost = current_cost + graph.NeighborCost(current, neighbor);
			if (auto neighbor_it = cost.Find(neighbor); cost.Found(neighbor_it) and new_cost >= neighbor_it->v) {
				continue;
			}
			
			frontier.Put(neighbor, new_cost + goal.EuclidDistanceHeuristic(neighbor));
			cost[neighbor] = new_cost;
			came_from[neighbor] = current;
		}
	}

	if (not complete) {
		return Vector<GridCell>::Null();
	}

	auto path = Vector<GridCell>(goal.ManhattanDistanceHeuristic(start).v);
	auto current = goal;
	while (current != start and graph.IsValid(current)) { 
		path.PushBack(current);
		current = came_from.Find(current)->v;
	}
	
	path.PushBack(start);
	path.Reverse();

	return path;
}
