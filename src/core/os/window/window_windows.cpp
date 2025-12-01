void _Present() {}

Events 
_WaitAndGetNextEvent() {
	MSG msg;
	//TODO(Jesse): Filter some message types from the queue.
	if (GetMessage(&msg, nullptr, 0, 0) == -1) {
		PostQuitMessage(0);
		return Events::Nil;
	}

	TranslateMessage(&msg); 
	return (Events)DispatchMessage(&msg);
} 

void 
_DestroyWindow() {
	if (main_window_handle != nullptr) {
		DestroyWindow(main_window_handle);
	}

	main_window_handle = nullptr;
}

LRESULT CALLBACK internal
_WindowMessageProc(HWND hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) {
	switch (Msg) {
		case WM_KEYUP: 
		case WM_KEYDOWN: {
			u32 repeat_count = LParam & 0xFFFF;
			bool was_down = (LParam >> 30) & 1;

			if (not was_down or (repeat_count > 0)) {
				if (WParam == 'A') {
					return (LRESULT)Events::AKey;
				}
				else if (WParam == 'W') {
					return (LRESULT)Events::WKey;
				}
				else if (WParam == 'S') {
					return (LRESULT)Events::SKey;
				}
				else if (WParam == 'D') {
					return (LRESULT)Events::DKey;
				} 
				else if (WParam == VK_ESCAPE) {
					return (LRESULT)Events::EscapeKey;
				}
			}
		}

		case WM_DESTROY: {
			PostQuitMessage(0);
			return (LRESULT)Events::Nil;
		} break;

		case WM_CLOSE: {
			return (LRESULT)Events::Nil;
		} break;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			return (LRESULT)Events::Unsupported;
		} break;
	}

	DefWindowProc(hwnd, Msg, WParam, LParam);
	return (LRESULT)Events::Unsupported;
}

void 
_OpenWindow() {
	WNDCLASS wc = {};
	wc.lpfnWndProc   = _WindowMessageProc;
	wc.hInstance     = HINST_THISCOMPONENT;
	wc.lpszClassName = "Norns Window Class";
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
	RegisterClass(&wc);

	main_window_handle = CreateWindow("MainWndClass", "Norns", WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
		(HWND)NULL, (HMENU)NULL, HINST_THISCOMPONENT, (LPVOID)NULL); 
	assert(main_window_handle != nullptr);

	ShowWindow(main_window_handle, SW_NORMAL); 
}

HWND main_window_handle = {};
