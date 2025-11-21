void* game(void *payload) {
	auto& surface = ((GameInitializePayload*)payload)->surface;
	auto& surface_pixels = surface->GetSurfacePixels();

	auto& window_event_queue = ((GameInitializePayload*)payload)->window_event_queue;

	bool running = true;
	while (running) {
		auto timer = os::Time::Now();
		if (window_event_queue->HasItems()) {
			auto e = window_event_queue->Pop();
			logger << e;

			if (e == os::Window::Events::Nil or e == os::Window::Events::EscapeKey) {
				running = false;
			}

			if (e == os::Window::Events::Presented) { //NOTE(Jesse): VSYNC blank has occured, start next frame!
				u16 surface_height = surface->GetHeight();
				u16 surface_width = surface->GetWidth();
				for (u32 y = 0; y < surface_height; ++y) {
					for (u32 x = 0; x < surface_width; ++x) {
						surface_pixels[y * surface_width + x] = {timer * 0.000001, timer * 0.000001, timer * 0.000001, 0};
					}
				}

				surface->Present();
			}
		}
	}
	
	return 0;
}