#include "hack.h"

LRESULT CALLBACK handle_platform_messages_(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

static inline void initialize_main_window_(HINSTANCE instance)
{
	{
		WNDCLASSEXA wc =
		{
			.cbSize = sizeof(wc),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = handle_platform_messages_,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = instance,
			.hIcon = 0,
			.hCursor = 0,
			.hbrBackground = (HBRUSH)COLOR_BACKGROUND,
			.lpszMenuName = 0,
			.lpszClassName = g_default_name,
			.hIconSm = 0,
		};
		(void)RegisterClassExA(&wc);
	}

	g_window.handle = (window *)CreateWindowExA(
		WS_EX_OVERLAPPEDWINDOW,
		g_default_name,
		g_default_name,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, 0, 0
	);
	if (!g_window.handle) FUCK(-1, "Failed to create the main window");
	set_window_show_state(g_window.handle, window_show_state_MAXIMIZED);
}

static inline void retrieve_messages_()
{
	static MSG s_msg;
	(void)GetMessageA(&s_msg, 0, 0, 0);
	(void)TranslateMessage(&s_msg);
	(void)DispatchMessageA(&s_msg);
}

static inline void count_iteration_()
{
	u64 time = get_time();
	if (time - g_iteration.start_time >= /* 1 second */ 10'000'000)
	{
		g_iteration.rate = g_iteration.count;
		g_iteration.count = 0;
		g_iteration.start_time = time;
	}
	++g_iteration.count;
}

static inline void swap_surface_()
{
	UNIMPLEMENTED();
}

static inline void resize_surface_()
{
	UNIMPLEMENTED();
}

static inline void render_()
{
	UNIMPLEMENTED();
}

//	================================================================================
//	# Main
//	================================================================================

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR cmdline, int cmdshow)
{
	initialize_main_window_(instance);

	while (g_flags.executing)
	{
		retrieve_messages_();

		if (g_window.resized)
		{
			g_window.render = 1;
			resize_surface_();
		}
		if (g_window.render)
		{
			render_();
			swap_surface_();
		}

		count_iteration_();
	}

	return 0;
}

static inline void handle_character_input_message_(utf32 character)
{
	if (g_input.flags == input_flag_NONE)
	{
		push_character(character);
	}
	else
	{
		Callback callback = g_input.map[g_input.flags][character];
		if (callback)
		{
			if (callback(character)) REPORT_ERROR("Failed to execute callback");
		}
		else REPORT_ERROR("Callback inexists");
	}
}

LRESULT CALLBACK handle_platform_messages_(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		set_window_show_state(g_window.handle, window_show_state_MINIMIZED);
		g_window.show_state = window_show_state_MINIMIZED;
		return 0;

	case WM_DESTROY:
		g_flags.executing = 0;
		PostQuitMessage(0);
		return 0;

		// NOTE(Emhyr): If the window is resized, resize the fucking surface.

		//	================================================================================
		//	# Input Handlers
		//
		//	Executes the callback that's translated from inputs. 
		//
		//	## Process
		//
		//	Retrieve > Translate > Execute
		//
		//	NOTE(Emhyr): `Input_Flag_SHIFT` is useless because `Input_Flag_CTRL` needs to be
		//	set for it to become operative.
		//	================================================================================
	case WM_KEYUP:
	case WM_SYSKEYUP:
		switch (wparam)
		{
		case VK_CONTROL:
		case VK_MENU:
			g_input.flags &= ~(0x01 << (wparam - 0x11));
			break;
		}
		return 0;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		switch (wparam)
		{
		case VK_CONTROL:
		case VK_MENU:
			g_input.flags |= 0x01 << (wparam - 0x11);
			break;
		case VK_CAPITAL:
			g_input.flags ^= input_flag_CAPS;
			break;
		}
		return 0;
	case WM_CHAR:
		handle_character_input_message_(wparam);
		return 0;
	default:
		return DefWindowProc(wnd, msg, wparam, lparam);
	}

	UNREACHABLE();
}

//	================================================================================
//	# Utilities 
//	================================================================================

void copy(u64 count, const void *destination, const void *source)
{
	(void)__builtin_memcpy((void *)destination, (void *)source, count);
}

u64 v_format(u64 size, char *buffer, const char *format, va_list vargs)
{
	return __builtin_vsnprintf(buffer, size, format, vargs);
}

[[gnu::format(printf, 3, 4)]]
u64 format(u64 size, char *buffer, const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	u64 result = v_format(size, buffer, fmt, vargs);
	va_end(vargs);
	return result; 
}

//	================================================================================
//	# System 
//	================================================================================

void sleep(u32 milliseconds)
{
	Sleep(milliseconds);
}

u64 get_time()
{
	u64 result;
	GetSystemTimeAsFileTime((LPFILETIME)&result);
	return result;
}

//	================================================================================
//	# Diagnostics 
//	================================================================================

void v_report(const char *function, unsigned line, report_level level, const char *fmt, va_list vargs)
{
	char buf[4096];
	const char *str;
	u64 off;
	switch (level)
	{
	case report_level_NONE:
		UNREACHABLE();
		break;
	case report_level_NOTE:
		off = sizeof("NOTE:");
		str = "NOTE:";
		break;
	case report_level_WARNING:
		off = sizeof("WARNING:");
		str = "WARNING:";
		break;
	case report_level_ERROR:
		off = sizeof("ERROR:");
		str = "ERROR:";
		break;
	}
	copy(off, buf, str);
	off += format(sizeof(buf) - off, buf + off - 1, "%s:%u:", function, line);
	off += v_format(sizeof(buf) - off, buf + off - 1, fmt, vargs);
	OutputDebugStringA(buf);
}

void report(const char *function, unsigned line, report_level level, const char *format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	v_report(function, line, level, format, vargs);
	va_end(vargs);
}

void fuck(const char *function, unsigned line, int exit_code, const char *format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	v_report(function, line, report_level_ERROR, format, vargs);
	va_end(vargs);
	exit(exit_code);
}

//	================================================================================
//	# Window Handling
//	================================================================================

u8 set_window_show_state(window *wnd, window_show_state state)
{
	return ShowWindow((HWND)wnd, state);
}

//	================================================================================
//	# Input Handling
//	================================================================================

//	================================================================================
//	# Editor
//	================================================================================

//	================================================================================
//	# Graphics
//	================================================================================

//	================================================================================
//	# Globals
//	================================================================================

decltype(g_window)		   g_window;
decltype(g_input)		   g_input;
decltype(g_iteration) g_iteration;
decltype(g_flags)		   g_flags;
