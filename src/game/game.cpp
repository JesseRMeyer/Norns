#include "path_finding/a_star.cpp"

struct GameState {
	u16 MammaDuck;
	u16 BabyDucks[12];
};

void 
Process(os::Surface& surface, PCG32Uni01& rng_ctx, Grid2D<u8>& cost_grid, GameState& game_state) {
	//auto now = os::Time::Now();

	//u16 surface_height = surface.GetHeight();
	u16 surface_width = surface.GetWidth();
	auto& surface_pixels = surface.GetSurfacePixels();

	GridCell start = {
		0, 0,
	};

	GridCell goal = {
		64, 64,
	};

	auto path = AStar(cost_grid, start, goal);
	assert(path[0] == start);

	for (u32 y = 0; y < cost_grid.GetY(); ++y) {
		for (u32 x = 0; x < cost_grid.GetX(); ++x) {
			u8 cell_cost = cost_grid[GridCell{x, y}];
			surface_pixels[y * surface_width + x] = {cell_cost * 25, cell_cost * 25, cell_cost * 25, 0};
		}
	}

	for (auto& p_xy: path) {
		surface_pixels[p_xy.y * surface_width + p_xy.x] = {0, 255, 255, 0};
	}

	surface_pixels[start.y * surface_width + start.x] = {0, 0, 255, 0};
	surface_pixels[goal.y * surface_width + goal.x] = {0, 255, 0, 0};

	for (u16 y = 0; y < cost_grid.GetY(); ++y) {
		for (u16 x = 0; x < cost_grid.GetX(); ++x) {
			f32 cost_01 = ((clamp(SampleStandardGaussian(rng_ctx), -8.0f, 8.0f) / 8.0f) + 1.0f) * 0.5f;
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
	PCG32Uni01 rng_ctx{};

	//NOTE(Jesse): Initialize grid cost memory with Gaussian Noise to stress test A*.
	u8 grid_memory[128][128] = {};
	auto cost_grid = Grid2D<u8>{(u8*)grid_memory, size(grid_memory[0]), size(grid_memory)};
	for (u16 y = 0; y < cost_grid.GetY(); ++y) {
		for (u16 x = 0; x < cost_grid.GetX(); ++x) {
			f32 cost_01 = ((clamp(SampleStandardGaussian(rng_ctx), -8.0f, 8.0f) / 8.0f) + 1.0f) * 0.5f;
			cost_grid[GridCell{x, y}] = u8(cost_01 * 10.0f + 0.5f);
		}
	}

	bool running = true;
	while (running) {
		window_event_queue->WaitForItem();
		
		auto e = window_event_queue->Pop();
		logger << e;

		if (e.Kind() == Event::Kind::Nil or 
		   (e.Kind() == Event::Kind::Keyboard and
			e.Key() == Event::Keyboard::Escape)) {
			running = false;
		}

		if (e.Kind() == Event::Kind::Presented) { //NOTE(Jesse): VSYNC blank has occured, start next frame!
			Process(*surface, rng_ctx, cost_grid, game_state);

			surface->Present();
		}
	}
	
	return 0;
}
