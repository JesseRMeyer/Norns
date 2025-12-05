#include "path_finding/a_star.cpp"

void Process(os::Surface& surface) {
	auto now = os::Time::Now();

	u16 surface_height = surface.GetHeight();
	u16 surface_width = surface.GetWidth();
	auto& surface_pixels = surface.GetSurfacePixels();

	for (u32 y = 0; y < surface_height; ++y) {
		for (u32 x = 0; x < surface_width; ++x) {
			surface_pixels[y * surface_width + x] = {now * 0.000001f, now * 0.000001f, now * 0.000001f, 0};
		}
	}

	GridCell goal = {
		25, 25,
	};

	GridCell start = {
		0, 0,
	};

	u8 grid_memory[256][256] = {};
	Grid2D<u8> cost_grid = {(u8*)grid_memory, size(grid_memory[0]), size(grid_memory)};

	for (u16 y = 0; y < size(grid_memory); ++y) {
		for (u16 x = 0; x < size(grid_memory[0]); ++x) {
			cost_grid[GridCell{x, y}] = y ^ x;
		}
	}

	auto reversed_path = AStar(cost_grid, start, goal);
}

void* Game(void *payload) {
	auto& surface = ((GameInitializePayload*)payload)->surface;
	auto& window_event_queue = ((GameInitializePayload*)payload)->window_event_queue;
	auto& logger = *((GameInitializePayload*)payload)->logger;

	bool running = true;
	while (running) {
		if (window_event_queue->HasItems()) {
			auto e = window_event_queue->Pop();
			logger << e;

			if (e == os::Window::Events::Nil or e == os::Window::Events::EscapeKey) {
				running = false;
			}

			if (e == os::Window::Events::Presented) { //NOTE(Jesse): VSYNC blank has occured, start next frame!
				Process(*surface);

				surface->Present();
			}
		} 

		CPU_PAUSE();
	}
	
	return 0;
}
