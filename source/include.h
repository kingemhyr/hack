#pragma once

#if defined _WIN32 || defined _WIN64
#	define PLATFORM_WINDOWS
#endif

#if defined PLATFORM_WINDOWS
#	include <Windows.h>
#	include <hidusage.h>

ULONG DbgPrint(PCSTR Format, ...);
#endif


#include <assert.h>
#include <memory.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
