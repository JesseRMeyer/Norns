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
	Surface_RGBA* shared_ptr = nullptr; //NOTE(Jesse): Is initialized by Window().
};

class Window {
public:
	using Event = void*;

	enum class Events {
		Nil,
		EscapeKey,
		Presented,
		AKey,
		WKey,
		SKey,
		DKey,
		Unsupported,
		Count,
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

	Events WaitAndGetNextEvent() {
		Event e = _WaitAndGetNextEvent();
		defer(_ReleaseEvent(e));

		return _ProcessEvent(e);
	}

	friend StringStream& 
	operator<<(StringStream& ss, const Events& e){
		char* str = nullptr;
		switch (e) {
			case Events::Nil: {
				str = "Nil";
			} break;

			case Events::Presented: {
				str = "Presented";
			} break;

			case Events::EscapeKey: {
				str = "EscapeKey";
			} break;

			case Events::AKey: {
				str = "AKey";
			} break;

			case Events::SKey: {
				str = "SKey";
			} break;

			case Events::WKey: {
				str = "WKey";
			} break;

			case Events::DKey: {
				str = "DKey";
			} break;

			case Events::Count: {
				str = "Count";
			} break;

			case Events::Unsupported: {
				str = "Unsupported";
			} break;

			default: {
				str = "Default";
			} break;
		}

		ss << str;
		return ss;
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

	u16 bytes_per_pixel = sizeof(Surface_RGBA);

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
