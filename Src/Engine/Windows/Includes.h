/*
	Any project that uses Windows should #include this file
	to ensure consistency
*/

#ifndef ENGINE_WINDOWS_INCLUDES_H
#define ENGINE_WINDOWS_INCLUDES_H

// Exclude extraneous Windows stuff
#define WIN32_LEAN_AND_MEAN
// Prevent Windows from creating min/max macros
#define NOMINMAX

	// Initialize Windows
	#include <Windows.h>

#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#endif	// ENGINE_WINDOWS_INCLUDES_H
