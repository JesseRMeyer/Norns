#include "path_finding/a_star.cpp"

union V2 {
	struct {
		f32 x;
		f32 y;
	};

	V2
	operator+(V2& other) {
		return {x + other.x, y + other.y};
	}

	V2
	operator+(V2&& other) {
		return {x + other.x, y + other.y};
	}

	V2
	operator-(V2& other) {
		return {x - other.x, y - other.y};
	}

	V2
	operator*(f32 other) {
		return {x * other, y * other};
	}

	V2
	operator/(f32 other) {
		other = 1.0f / other;
		return {x * other, y * other};
	}

	f32 e[2];
};

class Entity {
public:
	Entity(): ticks_per_second(60), coords({}), last_tick_time(os::Time::Now()) {}
	Entity(f32 ticks_per_second, V2 coords): ticks_per_second(ticks_per_second), coords(coords),  last_tick_time(os::Time::Now()) {}

	u8
	GetTicksRemaining(os::Time::Nano current_time) {
		f32 seconds_per_tick = 1.0f / ticks_per_second;

		auto dt = os::Time::NanoToSeconds(current_time - last_tick_time);
		auto ticks = dt / seconds_per_tick;
		if (ticks >= 1) {
			last_tick_time = current_time;
		}

		return ticks;
	}

	f32 const&
	GetX() {
		return coords.x;
	}

	f32 const&
	GetY() {
		return coords.y;
	}

	V2 const&
	GetXY() {
		return coords;
	}

	void
	SetXY(f32& x, f32& y) {
		coords = {x, y};
	}

	template <typename U>
	void
	SetXY(U&& xy) {
		coords = forward<U>(xy);
	}

private:
	f32 ticks_per_second;
	V2 coords;
	os::Time::Nano last_tick_time;
};

struct GameState {
	V2 mamma_duck;
	Entity ducklings[12];

	Slice<Entity>
	GetDucklings() {
		return Slice<Entity>(ducklings, size(ducklings));
	}
};

void 
Process(os::Surface& surface, PCG32Uni01& rng_Uni01, Grid2D<u8>& cost_grid, GameState& game_state) {
	auto now = os::Time::Now();

	//u16 surface_height = surface.GetHeight();
	u16 surface_width = surface.GetWidth();
	auto& surface_pixels = surface.GetSurfacePixels();

	for (u32 y = 0; y < cost_grid.GetY(); ++y) {
		for (u32 x = 0; x < cost_grid.GetX(); ++x) {
			u8 cell_cost = cost_grid[GridCell{x, y}];
			surface_pixels[y * surface_width + x] = {cell_cost * 25, cell_cost * 25, cell_cost * 25, 0};
		}
	}

	GridCell goal = { //NOTE(Jesse): OK the type casts here are out of control.
		clamp((u16)game_state.mamma_duck.x, 
			  (u16)0, 
			  (u16)((u16)cost_grid.GetX() - (u16)1)),
		clamp((u16)game_state.mamma_duck.y, 
			  (u16)0, 
			  (u16)((u16)cost_grid.GetY() - (u16)1)),
	};

	for (auto& duckling: game_state.GetDucklings()) {
		u8 ticks_remaining = duckling.GetTicksRemaining(now);
		if (ticks_remaining > 0) {
			GridCell start = {
				duckling.GetX(), duckling.GetY()
			};

			auto path = AStar(cost_grid, start, goal);
			assert(path[0] == start);

			for (auto& p_xy: path) {
				surface_pixels[p_xy.y * surface_width + p_xy.x] = {0, 255, 255, 0};
			}

			surface_pixels[start.y * surface_width + start.x] = {0, 0, 255, 0};

			//NOTE(Jesse): Randomly peturb duckling coordinate because they embody randomness
			// but really it's to prevent them from stacking.
			V2 jittered_position = {
				(rng_Uni01() * 2.0f) - 1.0f,
				(rng_Uni01() * 2.0f) - 1.0f,
			};

			//NOTE(Jesse): Use remaining ticks to "jump" ahead in the path.
			auto& p_node = path[clamp(ticks_remaining, (u8)0, (u8)(path.Size() - 1))];
			duckling.SetXY(V2{p_node.x, p_node.y});// + (jittered_position * 0.01f));
		}
	}
	
	surface_pixels[goal.y * surface_width + goal.x] = {0, 255, 0, 0};

	//NOTE(Jesse): "Update World"
	for (u16 y = 0; y < cost_grid.GetY(); ++y) {
		for (u16 x = 0; x < cost_grid.GetX(); ++x) {
			f32 cost_01 = ((clamp(SampleStandardGaussian(rng_Uni01), -8.0f, 8.0f) / 8.0f) + 1.0f) * 0.5f;
			cost_grid[GridCell{x, y}] = u8(cost_01 * 10.0f + 0.5f);
		}
	}
}

void* 
Game(void *payload) {
	using Event = os::Window::Event;

	auto& surface = ((GameInitializePayload*)payload)->surface;
	auto& window_event_queue = ((GameInitializePayload*)payload)->window_event_queue;
	auto& logger = *((GameInitializePayload*)payload)->logger;

	GameState game_state{};
	PCG32Uni01 rng_Uni01{};

	//NOTE(Jesse): Initialize grid cost memory with Gaussian Noise to stress test A*.
	u8 grid_memory[128][128] = {};
	auto cost_grid = Grid2D<u8>{(u8*)grid_memory, size(grid_memory[0]), size(grid_memory)};
	for (u16 y = 0; y < cost_grid.GetY(); ++y) {
		for (u16 x = 0; x < cost_grid.GetX(); ++x) {
			f32 cost_01 = ((clamp(SampleStandardGaussian(rng_Uni01), -8.0f, 8.0f) / 8.0f) + 1.0f) * 0.5f;
			cost_grid[GridCell{x, y}] = u8(cost_01 * 10.0f + 0.5f);
		}
	}

	for (auto& duckling: game_state.GetDucklings()) {
		duckling.SetXY(V2{
			rng_Uni01() * (cost_grid.GetX() - 1) + 0.5f,
			rng_Uni01() * (cost_grid.GetY() - 1) + 0.5f
		});
	}

	while (true) {
		window_event_queue->WaitForItem();
		
		auto e = window_event_queue->Pop();
		logger << e;

		if (e.Kind() == Event::Kind::Nil or 
		   (e.Kind() == Event::Kind::Keyboard and
			e.Key() == Event::Keyboard::Escape)) {
			break;
		}

		if (e.Kind() == Event::Kind::Mouse and e.MouseStatus() == Event::Mouse::Move) {
			auto mouse_coords = e.GetMouseCoords();
			game_state.mamma_duck.x = (f32)mouse_coords[0];
			game_state.mamma_duck.y = (f32)mouse_coords[1];
		}

		if (e.Kind() == Event::Kind::Presented) { //NOTE(Jesse): VSYNC blank has occured, start next frame!
			Process(*surface, rng_Uni01, cost_grid, game_state);

			surface->Present();
		}
	}
	
	return 0;
}
