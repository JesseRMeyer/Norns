inline void 
_OpenWindow() {
	connection = xcb_connect(nil, nil);
	assert(xcb_connection_has_error(connection) == 0);

	bool shared_pixmaps_supported = true;
	{ //NOTE(Jesse): Handle extension support.
		auto shm_ext = xcb_get_extension_data(connection, &xcb_shm_id);
		auto present_ext = xcb_get_extension_data(connection, &xcb_present_id);

		assert(shm_ext->present and "xcb shared memory ext not available.");
		assert(present_ext->present and "and present ext not available.");

		present_first_event = present_ext->first_event;

		if (not xcb_shm_query_version_reply(connection, xcb_shm_query_version_unchecked(connection), nullptr)->shared_pixmaps) {
			//NOTE(Jesse): Not sure why, maybe NV graphics driver allocates this on the GPU without CPU mapping?
			shared_pixmaps_supported = false;
		}
	}

	window = (xcb_window_t)xcb_generate_id(connection);
	screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

	{
		i32 value_mask = XCB_CW_EVENT_MASK | XCB_CW_BACK_PIXEL;
		i32 event_mask[2];
		event_mask[0] = screen->black_pixel;
		event_mask[1] = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_LEAVE_WINDOW \
						| XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_RESIZE_REDIRECT | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;

		xcb_create_window(connection, screen->root_depth, window, screen->root,
			//x: i16, y: i16, width: u16, height: u16, border_width: u16,
			256, 256, width, height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, &event_mask[0]);
		
		xcb_map_window(connection, window);
		xcb_flush(connection);
	}

	{
		gfx_ctx = xcb_generate_id(connection);
		u32 gc_values[] = {
			screen->black_pixel,
			screen->white_pixel,
			1,
		};

		xcb_create_gc(connection, gfx_ctx, window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_GRAPHICS_EXPOSURES, gc_values);
	}

	keys = xcb_key_symbols_alloc(connection);
	assert(keys != nil);

	auto& surface_pixels = surface.GetSurfacePixels();
	{ //NOTE(Jesse): Create shared memory segment between XCB and this application for the window bitmap
		size_t total_bytes_count = width * height * bytes_per_pixel; //NOTE(Jesse): Current bytes_per_pixel includes scanline padding
		shm_segment = xcb_generate_id(connection);
		auto seg_request_reply = xcb_shm_create_segment_reply(connection, xcb_shm_create_segment(connection, shm_segment, total_bytes_count, false), NULL);
		defer(free(seg_request_reply));

		assert(seg_request_reply and seg_request_reply->nfd == 1);

		int shm_fd = xcb_shm_create_segment_reply_fds(connection, seg_request_reply)[0];
		
		surface_pixels = (Surface_RGBA*)mmap(nullptr, total_bytes_count, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
		assert(surface_pixels != MAP_FAILED);

		xcb_flush(connection);
	}

	pixmap = xcb_generate_id(connection);
	if (shared_pixmaps_supported) {
		xcb_shm_create_pixmap(connection, pixmap, window, width, height, screen->root_depth, shm_segment, 0);
	} else {
		xcb_create_pixmap(connection, screen->root_depth, pixmap, window, width, height);
	}

	present_id = xcb_generate_id(connection);
	xcb_present_select_input(connection, present_id, window, XCB_PRESENT_EVENT_MASK_COMPLETE_NOTIFY | XCB_PRESENT_EVENT_MASK_IDLE_NOTIFY);

	{ //NOTE(Jesse): Initialize surface to white, and emit a Present to kick off the event loop.
		for (u32 y = 0; y < height; ++y) {
			for (u32 x = 0; x < width; ++x) {
				surface_pixels[y * width + x] = {255, 255, 255, 0};
			}
		}

		Present();
	}

	assert(not xcb_connection_has_error(connection));

	return;
}

inline void
_Present() {
	if (connection and surface.GetSurfacePixels()) {
		xcb_shm_put_image(connection, pixmap, gfx_ctx, width, height, 
			0, 0, width, height, 0, 0, screen->root_depth, 
			XCB_IMAGE_FORMAT_Z_PIXMAP, 0, shm_segment, 0);

		xcb_present_pixmap(connection, window, pixmap, image_presented_count++, XCB_NONE, XCB_NONE, 0, 0, XCB_NONE, XCB_NONE, XCB_NONE, XCB_PRESENT_OPTION_NONE, 0, 0, 0, 0, nil);

		xcb_flush(connection);
	}
}

inline void
_DestroyWindow() {
	//mouse_cursor_show()

	if (surface.GetSurfacePixels()) {
		xcb_shm_detach(connection, shm_segment);
		munmap(surface.GetSurfacePixels(), width * height * bytes_per_pixel);
	}

	xcb_free_gc(connection, gfx_ctx);
	xcb_free_pixmap(connection, pixmap);

	xcb_key_symbols_free(keys);
	xcb_unmap_window(connection, window);
	xcb_destroy_window(connection, window);
	xcb_disconnect(connection);

	keys = nullptr;
	connection = nullptr;
	window = 0;
	pixmap = {};

	gfx_ctx = 0;
	shm_segment = 0;
	surface.GetSurfacePixels() = nullptr;
}

inline Events
_ProcessEvent(const Event& event) {
	#define xcb_response_type_to_event_id(x) ((x)->response_type & ~0x80)
	 
	#define XCB_Shift_L  0xffe1
	#define XCB_Shift_R  0xffe2
	#define XCB_Control_L  0xffe3
	#define XCB_Control_R  0xffe4
	#define XCB_Caps_Lock  0xffe5
	#define XCB_Shift_Lock  0xffe6
	#define XCB_Tab  0xff09
	#define XCB_Return  0xff0d
	#define XCB_Escape  0xff1b
	#define XCB_Delete  0xffff
	#define XCB_BackSpace  0xff08
	#define NORN_XCB_ERROR 0

	if (event == nullptr) {
		return Events::Nil;
	}

	//NOTE(Jesse): XCB is just a poorly designed API.
	auto xcb_event = (xcb_generic_event_t*)event;
	switch (xcb_response_type_to_event_id(xcb_event)) {
		//case xcb.VISIBILITY_NOTIFY:
		//case xcb.CLIENT_MESSAGE:
		case NORN_XCB_ERROR: {
			auto xcb_error = (xcb_generic_error_t*)event;
			fprintf(stderr, "XCB ERROR %d: %s %s\n", xcb_error->error_code, xcb_event_get_error_label(xcb_error->error_code), xcb_event_get_request_label(xcb_error->major_code));
		} break;

		case XCB_GE_GENERIC: {;
			if (((xcb_ge_generic_event_t*)xcb_event)->event_type == XCB_PRESENT_COMPLETE_NOTIFY) {
				return Events::Presented;
			}
		} break;

		case XCB_CONFIGURE_NOTIFY: { 
			fprintf(stdout, "Notify!\n");
			//NOTE(Jesse): Window Resize and Move events
			//auto configure_event = (xcb_configure_notify_event_t*)xcb_event
			//global_window_frame.left = (u16)configure_event.x;
			//global_window_frame.top = (u16)configure_event.y;
			//global_window_frame.width = configure_event.width;
			//global_window_frame.height = configure_event.height;
			//global_window_frame.right = global_window_frame.left + global_window_frame.width;
			//global_window_frame.bottom = global_window_frame.top + global_window_frame.height;
		} break;

		case XCB_KEY_PRESS:
		case XCB_KEY_RELEASE: {
			auto key_press = (xcb_key_press_event_t*)xcb_event;
			auto keysym = xcb_key_symbols_get_keysym(keys, key_press->detail, 0);
			if (keysym == XCB_Escape) {
				return Events::EscapeKey;
			}

			switch (keysym) {
				case 'a': {
					return Events::AKey;
				} break;

				case 'w': {
					return Events::WKey;
				} break;

				case 's': {
					return Events::SKey;
				} break;

				case 'd': {
					return Events::DKey;
				} break;

				default: {
					return Events::Unsupported;
				}
			}

			//TODO(Jesse): return WindowEvents.UnsupportedKeyPress

			//auto i_s = xcb_keyboard_and_mouse_inputs_to_input_state[keysym]
			//global_user_input_queue[user_input_queue_count] = i_s
			//user_input_queue_count += 1
		}

		case XCB_FOCUS_OUT:
			//intrin.atomic_store(&global_application_active, false)
			//mouse_cursor_show()

		case XCB_FOCUS_IN:
			//intrin.atomic_store(&global_application_active, true)
			//mouse_cursor_hide()

		case XCB_EXPOSE:
			//expose_event := cast(^xcb.expose_event_t)xcb_event
			//intrin.atomic_store(&global_application_active, true)

		case XCB_RESIZE_REQUEST:
			//new_size := cast(^xcb.resize_request_event_t)xcb_event
	//
			//intrin.atomic_store(&global_window_frame.width, new_size.width)
			//intrin.atomic_store(&global_window_frame.height, new_size.height)
			//intrin.atomic_store(&global_window_frame.right, global_window_frame.left + global_window_frame.width)
			//intrin.atomic_store(&global_window_frame.bottom, global_window_frame.top + global_window_frame.height)

		case XCB_BUTTON_PRESS:
		case XCB_BUTTON_RELEASE:
			//button_press := cast(^xcb.button_press_event_t)xcb_event

		case XCB_LEAVE_NOTIFY: //NOTE(Jesse): Mouse left the window extents
			//NOTE(Jesse): This must be tracked otherwise the application will mistakenly believe
			// that the cursor is always within the application's window
			//mouse_move_event := cast(^xcb.leave_notify_event_t)xcb_event
			//new_cursor_pos := [2]u16 {
			//	cast(u16)mouse_move_event.root_x,
			//	cast(u16)mouse_move_event.root_y
			//}
			//intrin.atomic_store(&global_mouse_pos.x, new_cursor_pos.x)
			//intrin.atomic_store(&global_mouse_pos.y, new_cursor_pos.y)

		case XCB_MOTION_NOTIFY:
			//NOTE(Jesse): The reason we gather "push" mouse inputs through the event loop here
			// as opposed to a "pull" in the game loop is because the latency in the pull
			// appears quite severe even tho it is probably significantly less work.
			// For one, the loop only ever gets the most recent and therefore most relevant mouse location
			// when it needs it.  While here, we gather ALL mouse motion events.  I suspect
			// high polling mice generate a huge number of events so there is some churn here.
			// BUT, this event loop is in a separate thread so it won't slow down the game thread.

			//relative to origin root?

			//NOTE(Jesse): .state is a bitmask of buttons pressed while mouse was in motion
			//mouse_motion := cast(^xcb.motion_notify_event_t)xcb_event
			//new_cursor_pos := [2]u16 {
			//	cast(u16)mouse_motion.root_x,
			//	cast(u16)mouse_motion.root_y
			//}
			//intrin.atomic_store(&global_mouse_pos.x, new_cursor_pos.x)
			//intrin.atomic_store(&global_mouse_pos.y, new_cursor_pos.y)
		default: {} break;
	}

	//u32 window_values[4];
	//{
	//	(u32)global_window_frame.left, (u32)global_window_frame.top,
	//	(u32)global_window_frame.width, (u32)global_window_frame.height
	//}
	//xcb.configure_window(xcb_connection, xcb_window, {.X, .Y, .WIDTH, .HEIGHT}, &window_values[0])


#undef xcb_response_type_to_event_id

	return Events::Unsupported;
}

inline void
_ReleaseEvent(Event& event) {
	free(event);
	event = {};
}

inline Event
_WaitAndGetNextEvent() {
	return (Event)xcb_wait_for_event(connection);
}

xcb_window_t window = {};
xcb_screen_t* screen = nullptr;
xcb_connection_t* connection = nullptr;
xcb_key_symbols_t *keys = nullptr;

xcb_present_event_t present_id = {};
xcb_shm_seg_t shm_segment = {};
xcb_gcontext_t gfx_ctx = {};

xcb_pixmap_t pixmap = {};

u32 image_presented_count = 0;
u8 present_first_event = 0;