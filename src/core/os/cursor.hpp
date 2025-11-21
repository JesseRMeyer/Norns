class Cursor {
public:
	Cursor(Window& window) {
		_CreateInvisibible(window);
	}

	void CreateInvisibible(Window& window) {
		_CreateInvisibible(window);
	}

	void Hide(Window& window) {
		_Hide(window);
	}

	void Show(Window& window) {
		_Show(window);
	}

private:
	#include PLATFORM_CPP(cursor/cursor)
};