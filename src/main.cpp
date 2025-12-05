#include "core/core.hpp"

struct GameInitializePayload {
	SharedPtr<ThreadSafeFixedSizeQueue<os::Window::Events, os::Futex>> window_event_queue;
	SharedPtr<os::Surface> surface;
	SharedPtr<StringStream> logger;
};

#include "game/game.cpp"

int main() {
	auto window = os::Window();
	window.OpenWindow();

	auto cursor = os::Cursor(window);

	cursor.Hide(window);
	defer(cursor.Show(window));

	auto window_event_queue = SharedPtr<ThreadSafeFixedSizeQueue<os::Window::Events, os::Futex>>(256);

	auto logger = SharedPtr<StringStream>{};
	GameInitializePayload game_payload = {
		window_event_queue,
		window.GetSurface(),
		logger,
	};

	auto game_thread = os::Thread((void*)Game, &game_payload);

	bool running = true;
	while (running) {
		auto event = window.WaitAndGetNextEvent();
		window_event_queue->Put(event);

		if (event == os::Window::Events::Nil) {
			running = false;

			break;
		}
		else if (event == os::Window::Events::EscapeKey) {
			running = false;
			
			break;
		}

		if (not game_thread.IsAlive()) {
			running = false;
			
			break;
		}
	}

	return 0;
}
