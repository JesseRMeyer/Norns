inline void 
_CreateInvisibible(const Window& window) {
	xcb_pixmap_t empty_cursor_pix = (xcb_pixmap_t)xcb_generate_id(window.connection);
	xcb_create_pixmap(window.connection, 1, empty_cursor_pix, (xcb_drawable_t)window.screen->root, 1, 1);

	cursor = (xcb_cursor_t)xcb_generate_id(window.connection);

	xcb_create_cursor(window.connection, cursor, empty_cursor_pix, empty_cursor_pix, 0, 0, 0, 0, 0, 0, 0, 0);

	/*
	{ //NOTE(Jesse): Grab mouse position on application start
		auto qp_r = xcb_query_pointer_reply(window.connection, xcb_query_pointer(window.connection, window.window), 0);
		//global_mouse_pos = {
		//	cast(u16)qp_r.root_x,
		//	cast(u16)qp_r.root_y,
		//}
	}
	*/
}

inline void
_Hide(const Window& window) {
	xcb_change_window_attributes(window.connection, window.screen->root, XCB_CW_CURSOR, &cursor);
	xcb_flush(window.connection);
}

inline void
_Show(const Window& window) {
	xcb_cursor_t default_mouse; 
	xcb_change_window_attributes(window.connection, window.screen->root, XCB_CW_CURSOR, &default_mouse);
	xcb_flush(window.connection);
}

xcb_cursor_t cursor = {};
