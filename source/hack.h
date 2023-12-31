#pragma once

#include "include.h"

#define ASSERT(E) assert(E)
#define UNREACHABLE() ASSERT("UNREACHABLE" && 0)
#define UNIMPLEMENTED() ASSERT("UNIMPLEMENTED" && 0)

#define SOURCE_PATH "D:\\Projects\\hack"

//	================================================================================
//	# Types
//	================================================================================

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using wchar = wchar_t;
using utf8 = u8;
using utf32 = u32;

enum result : i64
{
	result_SUCCESS = 0,
	result_FAILURE = -1,	
	result_INEXIST = -2,
};

using Callback = result (*)(u64 arg);

//	================================================================================
//	# Constants
//	================================================================================

const char g_default_name[] = "HACK";

//	================================================================================
//	# Utilities 
//	================================================================================

static constexpr int MEMORY_PAGE_SIZE = 4096;

void copy(u64 count, const void *destination, const void *source);

void set(u64 count, const void *destination, u8 byte);

u64 v_format(u64 size, char *buffer, const char *format, va_list vargs);

[[gnu::format(printf, 3, 4)]]
u64 format(u64 size, char *buffer, const char *format, ...);

//	================================================================================
//	# System 
//	================================================================================

void sleep(u32 milliseconds);
u64 get_time();

void *allocate_virtual_memory(u64 size, void *address);
void deallocate_virtual_memory(u64 size, void *address);

//	================================================================================
//	# Diagnostics
//	================================================================================

enum report_level : u8
{
	report_level_NONE,
	report_level_NOTE,
	report_level_WARNING,
	report_level_ERROR,
};

void v_report(const char *function, unsigned line, report_level level, const char *format, va_list vargs);

[[gnu::format(printf, 4, 5)]]
void report(const char *function, unsigned line, report_level level, const char *format, ...);

[[noreturn, gnu::format(printf, 4, 5)]]
void fuck(const char *function, unsigned line, int exit_code, const char *format, ...);

#define REPORT(...) report(__FUNCTION__, __LINE__, __VA_ARGS__)
#define REPORT_NOTE(...) REPORT(report_level_NOTE, __VA_ARGS__)
#define REPORT_WARNING(...) REPORT(report_level_WARNING, __VA_ARGS__)
#define REPORT_ERROR(...) REPORT(report_level_ERROR, __VA_ARGS__)
#define FUCK(...) fuck(__FUNCTION__, __LINE__, __VA_ARGS__)

//	================================================================================
//	# Window Handling 
//	================================================================================

struct window;

enum window_show_state : u8
{
	window_show_state_HIDDEN = SW_HIDE,
	window_show_state_NORMAL = SW_NORMAL,
	window_show_state_MINIMIZED = SW_SHOWMINIMIZED,
	window_show_state_MAXIMIZED = SW_SHOWMAXIMIZED,
};

u8 set_window_show_state(window *window, window_show_state show_state);

//	================================================================================
//	# Input Handling
//	================================================================================

enum input_state
{
	input_state_NONE,
	input_state_WRITING,
	input_state_PROMPTED,
	ALL_INPUT_STATES = input_state_NONE | input_state_WRITING | input_state_PROMPTED,
};

enum input_flag : u8
{
	input_flag_NONE = 0x00,
	input_flag_CTRL = 0x01,
	input_flag_ALT = 0x02,
	input_flag_CAPS = 0x04,
	ALL_INPUT_FLAGS = input_flag_CTRL | input_flag_ALT | input_flag_CAPS,
};

// NOTE(Emhyr): Despite `Input_Flag` clearly being a C-like enum, why the flip
// can I not use these operators without overloading them?
constexpr input_flag operator|(const input_flag &l, const auto &r) { return (input_flag)((u8)l | (u8)r); }
constexpr input_flag operator&(const input_flag &l, const auto &r) { return (input_flag)((u8)l & (u8)r); }
constexpr input_flag operator^(const input_flag &l, const auto &r) { return (input_flag)((u8)l ^ (u8)r); }
constexpr input_flag &operator|=(input_flag &l, const auto &r) { return l = l | r; }
constexpr input_flag &operator&=(input_flag &l, const auto &r) { return l = l & r; }
constexpr input_flag &operator^=(input_flag &l, const auto &r) { return l = l ^ r; }

static constexpr input_flag ALL_INPUT_MODIFIERS = input_flag_CTRL | input_flag_ALT | input_flag_CAPS;

static constexpr int MAX_INPUT_KEY_VALUE = 254;

//	================================================================================
//	# Editor
//	================================================================================

struct buffer_page
{
	static constexpr int CAPACITY = 256;

	utf8 (*text)[CAPACITY];
	u8 mass;
	u8 lines;
	u8 lines_count;
	u8 mutated:1;
	u8 flags:1;
};

struct cursor
{
	buffer_page *page;
	u64 row;
	u64 column;
	u8 offset;
	u8 gap_length;
};

struct buffer
{
	buffer_page (*pages)[];
	u64 pages_count;
	u64 pages_capacity;
	u64 bytes_count;
	u64 lines_count;

	void *cursors; // NOTE(Emhyr): This is an SoA of `Cursor`.
	u64 cursors_count;
	u64 active_cursor_index;
};

struct editor
{
	static constexpr int MAX_BUFFER_COUNT = sizeof(buffer) / MEMORY_PAGE_SIZE;

	buffer *active_buffer;
	buffer (*buffers)[MAX_BUFFER_COUNT];
};

//	================================================================================
//	# Graphics
//	================================================================================

struct pixel
{
	u8 r, g, b;
};

struct color
{
	u8 r, g, b;
};

//	================================================================================
//	# Operations 
//	================================================================================

result push_character(u64 input);

//	================================================================================
//	# Globals
//	================================================================================

extern editor g_text_editor;

extern struct
{
	window *handle;
	window_show_state show_state;
	pixel *pixels;
	u64 width;
	u64 height;
	u8 render:1;
	u8 resized:1;
}
g_window;

extern struct
{
	input_state state;
	input_flag flags;
	Callback map[ALL_INPUT_MODIFIERS][MAX_INPUT_KEY_VALUE];
}
g_input;

extern struct
{
	u8 executing:1;
	u8 lever:1;
}
g_flags;

extern struct
{
	u64 count;
	u64 rate; // NOTE(Emhyr): Per second.
	u64 start_time;
}
g_iteration;
