/*
	This file can be included by a project that wants to use OpenGL
*/

#ifndef ENGINE_GRAPHICS_OPENGL_INCLUDES_H
#define ENGINE_GRAPHICS_OPENGL_INCLUDES_H

#if defined( WINDOWS_API )
	// windows.h _must_ be #included before GL.h
	#include "../../Windows/Includes.h"

	// The names of these two headers are standard,
	// but the location isn't
	#include <gl/GL.h>
	#include <gl/GLU.h>	// The "U" is for "utility functions"
#endif

// Modern OpenGL requires extensions
#include "../../../External/GLExtensions/GLExtensions.h"

#endif	// ENGINE_GRAPHICS_OPENGL_INCLUDES_H
