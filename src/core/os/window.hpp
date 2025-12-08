class Window;

union Surface_RGBA {
	struct { //NOTE(Jesse): Reflect byte order for assumed window backing formats.
		byte B, G, R, A; 
	};

	byte e[4];
};

class Surface {
public:
	explicit Surface(Window& parent): parent(parent), shared_ptr(nullptr) {}
	Surface_RGBA*& GetSurfacePixels() {
		return shared_ptr;
	}

	void Present();

	u16 GetHeight();
	u16 GetWidth();

private:
	Window& parent;
	Surface_RGBA* shared_ptr = nullptr; //NOTE(Jesse): Is initialized by Window::OpenWindow().
};

class Window {
public:
	class Event {
	public:
		using MouseCoords = u16[2];

		enum class Kind: u8 {
			Nil,
			Presented,
			Keyboard,
			Mouse,
			Unsupported,
			Count,
		};

		enum class Keyboard: u8 {
			Escape,
			A,
			W,
			S,
			D,
			Unsupported,
			Count,
		};

		enum class Mouse: u8 {
			Move,
			LeftButton,
			RightButton,
			Count,
		};

		Event(Kind k): kind(k) {
			//NOTE(Jesse): These kinds require payload
			assert(k != Kind::Keyboard);
			assert(k != Kind::Mouse);
		}

		Event(Keyboard k): kind(Kind::Keyboard), key(k) {}
		Event(MouseCoords& mc): kind(Kind::Mouse), mouse{Mouse::Move, {mc[0], mc[1]}} {}
		Event(Mouse m): kind(Kind::Mouse), mouse{m, {}} {}

		Event(): kind(Kind::Nil) {}

		Kind const& 
		Kind() {
			return kind;
		}

		Keyboard const&
		Key() {
			return key;
		}

		Mouse const& 
		MouseStatus() {
			return mouse.kind;
		}

		MouseCoords const&
		GetMouseCoords() {
			//NOTE(Jesse): These are corrected to stay bounded within
			// the window.
			return mouse.coords;
		}

		friend StringStream& 
		operator<<(StringStream& ss, Event const& e){
			char buffer[128] = {};
			char* str = nullptr;
			switch (e.kind) {
				case Kind::Nil: {
					str = "Nil";
				} break;

				case Kind::Presented: {
					str = "Presented";
				} break;

				case Kind::Keyboard: {
					int characters_written = stbsp_snprintf((char*)&buffer[0], size(buffer), "%s", "Keyboard: ");
					
					char* character = "";
					switch (e.key) {
						case Keyboard::A: {
							character = "A";
						} break;
						
						case Keyboard::S: {
							character = "S";
						} break;
						
						case Keyboard::D: {
							character = "D";
						} break;

						case Keyboard::W: {
							character = "W";
						} break;

						case Keyboard::Escape: {
							character = "Escape";
						} break;

						case Keyboard::Unsupported: {
							character = "Unsupported";
						}

						case Keyboard::Count:
						default: {
							character = "[ERROR] switch (e.key)";
						}
					}

					stbsp_snprintf((char*)&buffer[characters_written], size(buffer) - characters_written, "%s", character);
					str = &buffer[0];
				} break;

				case Kind::Mouse: {
					int characters_written = stbsp_snprintf((char*)&buffer[0], size(buffer), "%s", "Mouse: ");
					
					char* mouse_state = nullptr;
					switch (e.mouse.kind) {
						case Mouse::Move: {
							stbsp_snprintf((char*)&buffer[characters_written], size(buffer) - characters_written, "coords x: %d y: %d", e.mouse.coords[0], e.mouse.coords[1]);			
						} break;

						case Mouse::LeftButton: {
							mouse_state = "LeftButton";
						} break;
						
						case Mouse::RightButton: {
							mouse_state = "RightButton";
						} break;
						
						case Mouse::Count:
						default: {
							mouse_state = "[ERROR] e.mouse.kind";
						}
					}

					if (mouse_state != nullptr) {
						stbsp_snprintf((char*)&buffer[characters_written], size(buffer) - characters_written, "%s", mouse_state);
					}

					str = &buffer[0];
				} break;

				case Kind::Count: {
					str = "Count";
				} break;

				case Kind::Unsupported: {
					str = "Unsupported";
				} break;

				default: {
					str = "[ERROR] Default";
				} break;
			}

			ss << str;
			return ss;
		}

	private:
		enum Kind kind;

		union {
			Keyboard key;

			struct {
				Mouse kind;
				MouseCoords coords;
			} mouse;
		};
	};

	Window(): surface(*this) {}
	~Window() {
		_DestroyWindow();
	}

	//NOTE(Jesse): This method initializes the Window such that
	// Present() can be immediately called without any futher
	// setup by the user. 
	void OpenWindow() {
		return _OpenWindow();
	}

	Event WaitAndGetNextEvent() {
		return _WaitAndGetNextEvent();
	}

	void Present() {
		_Present();
	}

	Surface& GetSurface() {
		return surface;
	}

	u16 Height() {
		return height;
	}

	u16 Width() {
		return width;
	}

private:
	Surface surface;

	friend class Cursor;

	u16 width = 256;
	u16 height = 256;
	u16 top = 0;
	u16 left = 0;

	constexpr inline internal u32 buffer_count = 1; //TODO(Jesse): Double buffer?
	constexpr inline internal u16 bytes_per_pixel = sizeof(Surface_RGBA);

	#include PLATFORM_CPP(window/window)
};

void Surface::Present() {
	parent.Present();
}

u16 Surface::GetHeight() {
	return parent.Height();
}

u16 Surface::GetWidth() {
	return parent.Width();
}
