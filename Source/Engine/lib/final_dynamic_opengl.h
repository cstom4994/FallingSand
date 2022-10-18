/***
final_dynamic_opengl.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A open source single header file OpenGL-Loader C99 library.

This library is designed to load all the opengl functions for you so you can start right away with OpenGL up to version 4.6.
It even can create a rendering context for you, if needed.

Due to removing any kind of linking madness, all opengl functions are defined as static - so its private to this header file only!
Therefore you can use this library in one file only and cannot use in combination with other opengl libraries.

The only dependencies are built-in operating system libraries and a C99 complaint compiler.

Required linking is bare minimum:
	Win32: Link to kernel32.lib
	Unix/Linux: Link to ld.so

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your main C/C++ project and include it in one place you do the rendering.
- Define FGL_IMPLEMENTATION before including this header file in that translation unit.
- Load the library with fglLoadOpenGL(), while a opengl rendering context is already activated - or create a context using fglCreateOpenGLContext()
- Use all the OpenGL features you want
- Unload the library with fglUnloadOpenGL() when you are done
- Destroy the context when you created it using fglDestroyOpenGLContext()

-------------------------------------------------------------------------------
	Usage from a already activated rendering context
-------------------------------------------------------------------------------

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

if (fglLoadOpenGL(true)) {
	// ... load shader, whatever you want to do
	fglUnloadOpenGL();
}

-------------------------------------------------------------------------------
	Usage (Without a rendering context, but with an existing window)
-------------------------------------------------------------------------------

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

// Load opengl library without loading all the functions - functions are loaded separately later
if (fglLoadOpenGL(false)) {

	// Fill out window handle (This is platform dependent!)
	fglOpenGLContextCreationParameters contextCreationParams = {0};
#	if defined(FGL_PLATFORM_WIN32)
		contextCreationParams.windowHandle.win32.deviceContext = // ... pass your current device context here
		// or
		contextCreationParams.windowHandle.win32.windowHandle = // ... pass your current window handle here
#	endif

	// Create context and load opengl functions
	fglOpenGLContext glContext = {0};
	if (fglCreateOpenGLContext(&contextCreationParams, &glContext)) {
		fglLoadOpenGLFunctions();

		// ... load shader, whatever you want to do

		fglDestroyOpenGLContext(&glContext);
	}
	fglUnloadOpenGL();
}

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final Dynamic OpenGL is released under the following license:

MIT License

Copyright (c) 2017-2021 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

/*!
	\file final_dynamic_opengl.h
	\version v0.3.5.0 beta
	\author Torsten Spaete
	\brief Final Dynamic OpenGL (FGL) - A open source C99 single file header OpenGL-Loader library.
*/

// ****************************************************************************
//
// > Changelog
//
// ****************************************************************************

/*!
	\page page_changelog Changelog
	\tableofcontents

	# v0.4.0.0 beta:

	## Fixes:
	- Renamed fdyngl_api to fgl_api

	## Features:
	- Support for extern for all OpenGL API functions when FGL_AS_PRIVATE is unset

	## Changes:
	- New changelog format

	# v0.3.5.0 beta:
 	- Fixed: Fixed incompatibilities with MingW compiler (FARPROC)

    # v0.3.4.0 beta:
    - Fixed: Removed fglOpenGLState struct dependency for fgl__SetLastError()
    - Fixed: Removed fglOpenGLState struct dependency for Win32/X11 opengl functions
    - Fixed: GLX initialization was not working anymore

	# v0.3.3.0 beta:
	- Changed: Prevent including FGL before any other OpenGL library/header
	- Changed: Moved all platform specific code into its own block
	- Changed: fglOpenGLWindowHandle/fglOpenGLRenderingContext is now a union
	- Fixed: Corrected documentation errors
	- Fixed: Fixed fgl__ClearMemory was not working properly
	- New: [Win32] Implemented modern context creation

	# v0.3.2.0 beta:
	- Fixed: Fixed incompatibilities with C99

	# v0.3.1.0 beta:
	- Fixed: Fixed tons of compile errors on linux

	# v0.3.0.0 beta:
	- Changed: Transition from C++ to C99
	- Renamed fdyngl to fgl

	# v0.2.0.0 beta:
	- Changed: Added parameter for controlling to load of the extensions to LoadOpenGL()
	- Changed:
	- Fixed: Use stdint.h instead of inttypes.h
	- Fixed: WINGDIAPI detection was wrong
	- New: Added context creation support (CreateOpenGLContext, DestroyOpenGLContext)
	- New: Added LoadOpenGLFunctions()
	- New: Added PresentOpenGL()
	- New: Added GetLastError()
	- New: Written documentation

	# v0.1.0.0 beta:
	- Initial version
*/

/*!
	\page page_todo Todo
	\tableofcontents

	- [POSIX, GLX] Implement context creation

*/

// ****************************************************************************
//
// > Header
//
// ****************************************************************************
#ifndef FGL_INCLUDE_H
#define FGL_INCLUDE_H

//
// C99 detection
//
// https://en.wikipedia.org/wiki/C99#Version_detection
// Visual Studio 2015+
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
	//! Detected C99 compiler
#	define FGL_IS_C99
#elif defined(__cplusplus)
	//! Detected C++ compiler
#	define FGL_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

//
// Includes
//
#include <stddef.h> // ptrdiff_t
#include <stdint.h> // uint32_t
#include <stdlib.h> // NULL
#include <stdbool.h> // bool

#define fgl_null NULL

//! Macro for initialize a struct to zero
#if defined(FGL_IS_C99)
#	define FGL_ZERO_INIT {0}
#else
#	define FGL_ZERO_INIT {}
#endif

//
// Platform detection
//
#if defined(_WIN32)
#	define FGL_PLATFORM_WIN32
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FGL_PLATFORM_LINUX
#	define FGL_PLATFORM_POSIX
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FGL_PLATFORM_UNIX
#	define FGL_PLATFORM_POSIX
#else
#	error "This platform/compiler is not supported!"
#endif

//
// We do not support already active opengl headers/libraries
//
#if (defined(__gl_h_) || defined(__GL_H__) || defined(GL_VERSION_1_1)) || (defined(__glext_h_) || defined(GL_GLEXT_VERSION) || defined(GL_VERSION_1_2))
#	error "You cannot have any OpenGL library already included/loaded before using this library!"
#endif

//
// Required api defines
//
#ifdef FGL_PLATFORM_WIN32
#	ifdef APIENTRY
#		define FGL_GLAPIENTRY APIENTRY
#		define FGL_APIENTRY APIENTRY
#	else
#		if defined(__MINGW32__) || defined(__CYGWIN__) || (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
#			define FGL_APIENTRY __stdcall
#			ifndef FGL_GLAPIENTRY
#				define FGL_GLAPIENTRY __stdcall
#			endif
#			ifndef FGL_APIENTRY
#				define FGL_APIENTRY __stdcall
#			endif
#		else
#			define FGL_APIENTRY
#		endif
#	endif
#	ifdef WINGDIAPI
#		define FGL_WINGDIAPI WINGDIAPI
#	else
#		define FGL_WINGDIAPI __declspec(dllimport)
#	endif
#	if defined(__MINGW32__) || defined(__CYGWIN__)
#		define FGL_GLAPI extern
#	else
#		define FGL_GLAPI FGL_WINGDIAPI
#	endif
#else
#	define FGL_GLAPI extern
#	define FGL_APIENTRY
#endif

#if defined(FGL_AS_PRIVATE)
	//! API functions exported as static
#	define fgl_api static
#else
	//! API functions exported as extern
#	define fgl_api extern
#endif

//
// Platform includes
//
#if defined(FGL_PLATFORM_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN 1
#	endif
#	include <Windows.h>

//! Win32 OpenGL window handle
typedef struct fglWin32OpenGLWindowHandle {
	//! Window handle
	HWND windowHandle;
	//! Device context
	HDC deviceContext;
	//! Bool to indicate to release DC when done
	bool requireToReleaseDC;
} fglWin32OpenGLWindowHandle;

//! Win32 OpenGL rendering context
typedef struct fglWin32OpenGLRenderingContext {
	//! Rendering context
	HGLRC renderingContext;
} fglWin32OpenGLRenderingContext;

#elif defined(FGL_PLATFORM_POSIX)
#	include <dlfcn.h> // dlopen
#	include <X11/X.h>
#	include <X11/Xlib.h>
#	include <X11/Xutil.h> // XVisualInfo

//! Posix OpenGL window handle
typedef struct fglPosixOpenGLWindowHandle {
	//! Display
	Display *display;
	//! Window
	Window *window;
} fglPosixOpenGLWindowHandle;

typedef struct fglPosixOpenGLRenderingContext {
	//! Dummy
	int dummy;
} fglPosixOpenGLRenderingContext;
#endif

//
// API
//

#ifdef __cplusplus
extern "C" {
#endif

//! OpenGL profile type
	typedef enum fglOpenGLProfileType {
		//! No or legacy profile
		fglOpenGLProfileType_LegacyProfile = 0,
		//! Core profile
		fglOpenGLProfileType_CoreProfile,
		//! Compability profile
		fglOpenGLProfileType_CompabilityProfile,
	} fglOpenGLProfileType;

	//! OpenGL window handle
	typedef union fglOpenGLWindowHandle {
#if defined(FGL_PLATFORM_WIN32)
		//! Win32 window handle
		fglWin32OpenGLWindowHandle win32;
#elif defined(FGL_PLATFORM_POSIX)
		//! Posix window handle
		fglPosixOpenGLWindowHandle posix;
#endif
	} fglOpenGLWindowHandle;

	//! OpenGL window handle
	typedef union fglOpenGLRenderingContext {
#if defined(FGL_PLATFORM_WIN32)
		//! Win32 rendering context
		fglWin32OpenGLRenderingContext win32;
#elif defined(FGL_PLATFORM_POSIX)
		//! Posix rendering context
		fglPosixOpenGLRenderingContext posix;
#endif
	} fglOpenGLRenderingContext;

	//! OpenGL rendering context
	typedef struct fglOpenGLContext {
		//! Window handle container
		fglOpenGLWindowHandle windowHandle;
		//! Rendering context container
		fglOpenGLRenderingContext renderingContext;
		//! Is context valid
		bool isValid;
	} fglOpenGLContext;

	//! OpenGL Context Creation Parameters Container
	typedef struct fglOpenGLContextCreationParameters {
		//! Window handle
		fglOpenGLWindowHandle windowHandle;
		//! Desired major version
		uint32_t majorVersion;
		//! Desired minor version
		uint32_t minorVersion;
		//! Desired profile type
		fglOpenGLProfileType profile;
		//! Is forward compability enabled
		bool forwardCompability;
	} fglOpenGLContextCreationParameters;

	//! Sets the context parameters to default values
	fgl_api void fglSetDefaultOpenGLContextCreationParameters(fglOpenGLContextCreationParameters *outParams);

	//! Create a opengl context
	fgl_api bool fglCreateOpenGLContext(const fglOpenGLContextCreationParameters *contextCreationParams, fglOpenGLContext *outContext);

	//! Destroy the given opengl context
	fgl_api void fglDestroyOpenGLContext(fglOpenGLContext *context);

	//! Does all the things to get opengl up and running
	fgl_api bool fglLoadOpenGL(const bool loadFunctions);

	//! Releases all resources allocated for opengl
	fgl_api void fglUnloadOpenGL();

	//! Load all opengl functions
	fgl_api void fglLoadOpenGLFunctions();

	//! Presents the current frame for the given opengl context
	fgl_api void fglPresentOpenGL(const fglOpenGLContext *context);

	//! Returns last error string
	fgl_api const char *fglGetLastError();

	// *******************************************************************************
	//
	// > OpenGL Header
	//
	// Automatically generated. Do not modify by hand!
	//
	// *******************************************************************************
#	ifndef GL_VERSION_1_1
#		define GL_VERSION_1_1 1
		fgl_api bool isGL_VERSION_1_1;

		typedef unsigned int GLenum;
		typedef unsigned int GLbitfield;
		typedef unsigned int GLuint;
		typedef int GLint;
		typedef int GLsizei;
		typedef unsigned char GLboolean;
		typedef signed char GLbyte;
		typedef short GLshort;
		typedef unsigned char GLubyte;
		typedef unsigned short GLushort;
		typedef unsigned long GLulong;
		typedef float GLfloat;
		typedef float GLclampf;
		typedef double GLdouble;
		typedef double GLclampd;
		typedef void GLvoid;

#		define GL_ACCUM 0x0100
#		define GL_LOAD 0x0101
#		define GL_RETURN 0x0102
#		define GL_MULT 0x0103
#		define GL_ADD 0x0104
#		define GL_NEVER 0x0200
#		define GL_LESS 0x0201
#		define GL_EQUAL 0x0202
#		define GL_LEQUAL 0x0203
#		define GL_GREATER 0x0204
#		define GL_NOTEQUAL 0x0205
#		define GL_GEQUAL 0x0206
#		define GL_ALWAYS 0x0207
#		define GL_CURRENT_BIT 0x00000001
#		define GL_POINT_BIT 0x00000002
#		define GL_LINE_BIT 0x00000004
#		define GL_POLYGON_BIT 0x00000008
#		define GL_POLYGON_STIPPLE_BIT 0x00000010
#		define GL_PIXEL_MODE_BIT 0x00000020
#		define GL_LIGHTING_BIT 0x00000040
#		define GL_FOG_BIT 0x00000080
#		define GL_DEPTH_BUFFER_BIT 0x00000100
#		define GL_ACCUM_BUFFER_BIT 0x00000200
#		define GL_STENCIL_BUFFER_BIT 0x00000400
#		define GL_VIEWPORT_BIT 0x00000800
#		define GL_TRANSFORM_BIT 0x00001000
#		define GL_ENABLE_BIT 0x00002000
#		define GL_COLOR_BUFFER_BIT 0x00004000
#		define GL_HINT_BIT 0x00008000
#		define GL_EVAL_BIT 0x00010000
#		define GL_LIST_BIT 0x00020000
#		define GL_TEXTURE_BIT 0x00040000
#		define GL_SCISSOR_BIT 0x00080000
#		define GL_ALL_ATTRIB_BITS 0x000fffff
#		define GL_POINTS 0x0000
#		define GL_LINES 0x0001
#		define GL_LINE_LOOP 0x0002
#		define GL_LINE_STRIP 0x0003
#		define GL_TRIANGLES 0x0004
#		define GL_TRIANGLE_STRIP 0x0005
#		define GL_TRIANGLE_FAN 0x0006
#		define GL_QUADS 0x0007
#		define GL_QUAD_STRIP 0x0008
#		define GL_POLYGON 0x0009
#		define GL_ZERO 0
#		define GL_ONE 1
#		define GL_SRC_COLOR 0x0300
#		define GL_ONE_MINUS_SRC_COLOR 0x0301
#		define GL_SRC_ALPHA 0x0302
#		define GL_ONE_MINUS_SRC_ALPHA 0x0303
#		define GL_DST_ALPHA 0x0304
#		define GL_ONE_MINUS_DST_ALPHA 0x0305
#		define GL_DST_COLOR 0x0306
#		define GL_ONE_MINUS_DST_COLOR 0x0307
#		define GL_SRC_ALPHA_SATURATE 0x0308
#		define GL_TRUE 1
#		define GL_FALSE 0
#		define GL_CLIP_PLANE0 0x3000
#		define GL_CLIP_PLANE1 0x3001
#		define GL_CLIP_PLANE2 0x3002
#		define GL_CLIP_PLANE3 0x3003
#		define GL_CLIP_PLANE4 0x3004
#		define GL_CLIP_PLANE5 0x3005
#		define GL_BYTE 0x1400
#		define GL_UNSIGNED_BYTE 0x1401
#		define GL_SHORT 0x1402
#		define GL_UNSIGNED_SHORT 0x1403
#		define GL_INT 0x1404
#		define GL_UNSIGNED_INT 0x1405
#		define GL_FLOAT 0x1406
#		define GL_2_BYTES 0x1407
#		define GL_3_BYTES 0x1408
#		define GL_4_BYTES 0x1409
#		define GL_DOUBLE 0x140A
#		define GL_NONE 0
#		define GL_FRONT_LEFT 0x0400
#		define GL_FRONT_RIGHT 0x0401
#		define GL_BACK_LEFT 0x0402
#		define GL_BACK_RIGHT 0x0403
#		define GL_FRONT 0x0404
#		define GL_BACK 0x0405
#		define GL_LEFT 0x0406
#		define GL_RIGHT 0x0407
#		define GL_FRONT_AND_BACK 0x0408
#		define GL_AUX0 0x0409
#		define GL_AUX1 0x040A
#		define GL_AUX2 0x040B
#		define GL_AUX3 0x040C
#		define GL_NO_ERROR 0
#		define GL_INVALID_ENUM 0x0500
#		define GL_INVALID_VALUE 0x0501
#		define GL_INVALID_OPERATION 0x0502
#		define GL_STACK_OVERFLOW 0x0503
#		define GL_STACK_UNDERFLOW 0x0504
#		define GL_OUT_OF_MEMORY 0x0505
#		define GL_2D 0x0600
#		define GL_3D 0x0601
#		define GL_3D_COLOR 0x0602
#		define GL_3D_COLOR_TEXTURE 0x0603
#		define GL_4D_COLOR_TEXTURE 0x0604
#		define GL_PASS_THROUGH_TOKEN 0x0700
#		define GL_POINT_TOKEN 0x0701
#		define GL_LINE_TOKEN 0x0702
#		define GL_POLYGON_TOKEN 0x0703
#		define GL_BITMAP_TOKEN 0x0704
#		define GL_DRAW_PIXEL_TOKEN 0x0705
#		define GL_COPY_PIXEL_TOKEN 0x0706
#		define GL_LINE_RESET_TOKEN 0x0707
#		define GL_EXP 0x0800
#		define GL_EXP2 0x0801
#		define GL_CW 0x0900
#		define GL_CCW 0x0901
#		define GL_COEFF 0x0A00
#		define GL_ORDER 0x0A01
#		define GL_DOMAIN 0x0A02
#		define GL_CURRENT_COLOR 0x0B00
#		define GL_CURRENT_INDEX 0x0B01
#		define GL_CURRENT_NORMAL 0x0B02
#		define GL_CURRENT_TEXTURE_COORDS 0x0B03
#		define GL_CURRENT_RASTER_COLOR 0x0B04
#		define GL_CURRENT_RASTER_INDEX 0x0B05
#		define GL_CURRENT_RASTER_TEXTURE_COORDS 0x0B06
#		define GL_CURRENT_RASTER_POSITION 0x0B07
#		define GL_CURRENT_RASTER_POSITION_VALID 0x0B08
#		define GL_CURRENT_RASTER_DISTANCE 0x0B09
#		define GL_POINT_SMOOTH 0x0B10
#		define GL_POINT_SIZE 0x0B11
#		define GL_POINT_SIZE_RANGE 0x0B12
#		define GL_POINT_SIZE_GRANULARITY 0x0B13
#		define GL_LINE_SMOOTH 0x0B20
#		define GL_LINE_WIDTH 0x0B21
#		define GL_LINE_WIDTH_RANGE 0x0B22
#		define GL_LINE_WIDTH_GRANULARITY 0x0B23
#		define GL_LINE_STIPPLE 0x0B24
#		define GL_LINE_STIPPLE_PATTERN 0x0B25
#		define GL_LINE_STIPPLE_REPEAT 0x0B26
#		define GL_LIST_MODE 0x0B30
#		define GL_MAX_LIST_NESTING 0x0B31
#		define GL_LIST_BASE 0x0B32
#		define GL_LIST_INDEX 0x0B33
#		define GL_POLYGON_MODE 0x0B40
#		define GL_POLYGON_SMOOTH 0x0B41
#		define GL_POLYGON_STIPPLE 0x0B42
#		define GL_EDGE_FLAG 0x0B43
#		define GL_CULL_FACE 0x0B44
#		define GL_CULL_FACE_MODE 0x0B45
#		define GL_FRONT_FACE 0x0B46
#		define GL_LIGHTING 0x0B50
#		define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#		define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#		define GL_LIGHT_MODEL_AMBIENT 0x0B53
#		define GL_SHADE_MODEL 0x0B54
#		define GL_COLOR_MATERIAL_FACE 0x0B55
#		define GL_COLOR_MATERIAL_PARAMETER 0x0B56
#		define GL_COLOR_MATERIAL 0x0B57
#		define GL_FOG 0x0B60
#		define GL_FOG_INDEX 0x0B61
#		define GL_FOG_DENSITY 0x0B62
#		define GL_FOG_START 0x0B63
#		define GL_FOG_END 0x0B64
#		define GL_FOG_MODE 0x0B65
#		define GL_FOG_COLOR 0x0B66
#		define GL_DEPTH_RANGE 0x0B70
#		define GL_DEPTH_TEST 0x0B71
#		define GL_DEPTH_WRITEMASK 0x0B72
#		define GL_DEPTH_CLEAR_VALUE 0x0B73
#		define GL_DEPTH_FUNC 0x0B74
#		define GL_ACCUM_CLEAR_VALUE 0x0B80
#		define GL_STENCIL_TEST 0x0B90
#		define GL_STENCIL_CLEAR_VALUE 0x0B91
#		define GL_STENCIL_FUNC 0x0B92
#		define GL_STENCIL_VALUE_MASK 0x0B93
#		define GL_STENCIL_FAIL 0x0B94
#		define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#		define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#		define GL_STENCIL_REF 0x0B97
#		define GL_STENCIL_WRITEMASK 0x0B98
#		define GL_MATRIX_MODE 0x0BA0
#		define GL_NORMALIZE 0x0BA1
#		define GL_VIEWPORT 0x0BA2
#		define GL_MODELVIEW_STACK_DEPTH 0x0BA3
#		define GL_PROJECTION_STACK_DEPTH 0x0BA4
#		define GL_TEXTURE_STACK_DEPTH 0x0BA5
#		define GL_MODELVIEW_MATRIX 0x0BA6
#		define GL_PROJECTION_MATRIX 0x0BA7
#		define GL_TEXTURE_MATRIX 0x0BA8
#		define GL_ATTRIB_STACK_DEPTH 0x0BB0
#		define GL_CLIENT_ATTRIB_STACK_DEPTH 0x0BB1
#		define GL_ALPHA_TEST 0x0BC0
#		define GL_ALPHA_TEST_FUNC 0x0BC1
#		define GL_ALPHA_TEST_REF 0x0BC2
#		define GL_DITHER 0x0BD0
#		define GL_BLEND_DST 0x0BE0
#		define GL_BLEND_SRC 0x0BE1
#		define GL_BLEND 0x0BE2
#		define GL_LOGIC_OP_MODE 0x0BF0
#		define GL_INDEX_LOGIC_OP 0x0BF1
#		define GL_COLOR_LOGIC_OP 0x0BF2
#		define GL_AUX_BUFFERS 0x0C00
#		define GL_DRAW_BUFFER 0x0C01
#		define GL_READ_BUFFER 0x0C02
#		define GL_SCISSOR_BOX 0x0C10
#		define GL_SCISSOR_TEST 0x0C11
#		define GL_INDEX_CLEAR_VALUE 0x0C20
#		define GL_INDEX_WRITEMASK 0x0C21
#		define GL_COLOR_CLEAR_VALUE 0x0C22
#		define GL_COLOR_WRITEMASK 0x0C23
#		define GL_INDEX_MODE 0x0C30
#		define GL_RGBA_MODE 0x0C31
#		define GL_DOUBLEBUFFER 0x0C32
#		define GL_STEREO 0x0C33
#		define GL_RENDER_MODE 0x0C40
#		define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#		define GL_POINT_SMOOTH_HINT 0x0C51
#		define GL_LINE_SMOOTH_HINT 0x0C52
#		define GL_POLYGON_SMOOTH_HINT 0x0C53
#		define GL_FOG_HINT 0x0C54
#		define GL_TEXTURE_GEN_S 0x0C60
#		define GL_TEXTURE_GEN_T 0x0C61
#		define GL_TEXTURE_GEN_R 0x0C62
#		define GL_TEXTURE_GEN_Q 0x0C63
#		define GL_PIXEL_MAP_I_TO_I 0x0C70
#		define GL_PIXEL_MAP_S_TO_S 0x0C71
#		define GL_PIXEL_MAP_I_TO_R 0x0C72
#		define GL_PIXEL_MAP_I_TO_G 0x0C73
#		define GL_PIXEL_MAP_I_TO_B 0x0C74
#		define GL_PIXEL_MAP_I_TO_A 0x0C75
#		define GL_PIXEL_MAP_R_TO_R 0x0C76
#		define GL_PIXEL_MAP_G_TO_G 0x0C77
#		define GL_PIXEL_MAP_B_TO_B 0x0C78
#		define GL_PIXEL_MAP_A_TO_A 0x0C79
#		define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#		define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#		define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#		define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#		define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#		define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#		define GL_PIXEL_MAP_R_TO_R_SIZE 0x0CB6
#		define GL_PIXEL_MAP_G_TO_G_SIZE 0x0CB7
#		define GL_PIXEL_MAP_B_TO_B_SIZE 0x0CB8
#		define GL_PIXEL_MAP_A_TO_A_SIZE 0x0CB9
#		define GL_UNPACK_SWAP_BYTES 0x0CF0
#		define GL_UNPACK_LSB_FIRST 0x0CF1
#		define GL_UNPACK_ROW_LENGTH 0x0CF2
#		define GL_UNPACK_SKIP_ROWS 0x0CF3
#		define GL_UNPACK_SKIP_PIXELS 0x0CF4
#		define GL_UNPACK_ALIGNMENT 0x0CF5
#		define GL_PACK_SWAP_BYTES 0x0D00
#		define GL_PACK_LSB_FIRST 0x0D01
#		define GL_PACK_ROW_LENGTH 0x0D02
#		define GL_PACK_SKIP_ROWS 0x0D03
#		define GL_PACK_SKIP_PIXELS 0x0D04
#		define GL_PACK_ALIGNMENT 0x0D05
#		define GL_MAP_COLOR 0x0D10
#		define GL_MAP_STENCIL 0x0D11
#		define GL_INDEX_SHIFT 0x0D12
#		define GL_INDEX_OFFSET 0x0D13
#		define GL_RED_SCALE 0x0D14
#		define GL_RED_BIAS 0x0D15
#		define GL_ZOOM_X 0x0D16
#		define GL_ZOOM_Y 0x0D17
#		define GL_GREEN_SCALE 0x0D18
#		define GL_GREEN_BIAS 0x0D19
#		define GL_BLUE_SCALE 0x0D1A
#		define GL_BLUE_BIAS 0x0D1B
#		define GL_ALPHA_SCALE 0x0D1C
#		define GL_ALPHA_BIAS 0x0D1D
#		define GL_DEPTH_SCALE 0x0D1E
#		define GL_DEPTH_BIAS 0x0D1F
#		define GL_MAX_EVAL_ORDER 0x0D30
#		define GL_MAX_LIGHTS 0x0D31
#		define GL_MAX_CLIP_PLANES 0x0D32
#		define GL_MAX_TEXTURE_SIZE 0x0D33
#		define GL_MAX_PIXEL_MAP_TABLE 0x0D34
#		define GL_MAX_ATTRIB_STACK_DEPTH 0x0D35
#		define GL_MAX_MODELVIEW_STACK_DEPTH 0x0D36
#		define GL_MAX_NAME_STACK_DEPTH 0x0D37
#		define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38
#		define GL_MAX_TEXTURE_STACK_DEPTH 0x0D39
#		define GL_MAX_VIEWPORT_DIMS 0x0D3A
#		define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH 0x0D3B
#		define GL_SUBPIXEL_BITS 0x0D50
#		define GL_INDEX_BITS 0x0D51
#		define GL_RED_BITS 0x0D52
#		define GL_GREEN_BITS 0x0D53
#		define GL_BLUE_BITS 0x0D54
#		define GL_ALPHA_BITS 0x0D55
#		define GL_DEPTH_BITS 0x0D56
#		define GL_STENCIL_BITS 0x0D57
#		define GL_ACCUM_RED_BITS 0x0D58
#		define GL_ACCUM_GREEN_BITS 0x0D59
#		define GL_ACCUM_BLUE_BITS 0x0D5A
#		define GL_ACCUM_ALPHA_BITS 0x0D5B
#		define GL_NAME_STACK_DEPTH 0x0D70
#		define GL_AUTO_NORMAL 0x0D80
#		define GL_MAP1_COLOR_4 0x0D90
#		define GL_MAP1_INDEX 0x0D91
#		define GL_MAP1_NORMAL 0x0D92
#		define GL_MAP1_TEXTURE_COORD_1 0x0D93
#		define GL_MAP1_TEXTURE_COORD_2 0x0D94
#		define GL_MAP1_TEXTURE_COORD_3 0x0D95
#		define GL_MAP1_TEXTURE_COORD_4 0x0D96
#		define GL_MAP1_VERTEX_3 0x0D97
#		define GL_MAP1_VERTEX_4 0x0D98
#		define GL_MAP2_COLOR_4 0x0DB0
#		define GL_MAP2_INDEX 0x0DB1
#		define GL_MAP2_NORMAL 0x0DB2
#		define GL_MAP2_TEXTURE_COORD_1 0x0DB3
#		define GL_MAP2_TEXTURE_COORD_2 0x0DB4
#		define GL_MAP2_TEXTURE_COORD_3 0x0DB5
#		define GL_MAP2_TEXTURE_COORD_4 0x0DB6
#		define GL_MAP2_VERTEX_3 0x0DB7
#		define GL_MAP2_VERTEX_4 0x0DB8
#		define GL_MAP1_GRID_DOMAIN 0x0DD0
#		define GL_MAP1_GRID_SEGMENTS 0x0DD1
#		define GL_MAP2_GRID_DOMAIN 0x0DD2
#		define GL_MAP2_GRID_SEGMENTS 0x0DD3
#		define GL_TEXTURE_1D 0x0DE0
#		define GL_TEXTURE_2D 0x0DE1
#		define GL_FEEDBACK_BUFFER_POINTER 0x0DF0
#		define GL_FEEDBACK_BUFFER_SIZE 0x0DF1
#		define GL_FEEDBACK_BUFFER_TYPE 0x0DF2
#		define GL_SELECTION_BUFFER_POINTER 0x0DF3
#		define GL_SELECTION_BUFFER_SIZE 0x0DF4
#		define GL_TEXTURE_WIDTH 0x1000
#		define GL_TEXTURE_HEIGHT 0x1001
#		define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#		define GL_TEXTURE_BORDER_COLOR 0x1004
#		define GL_TEXTURE_BORDER 0x1005
#		define GL_DONT_CARE 0x1100
#		define GL_FASTEST 0x1101
#		define GL_NICEST 0x1102
#		define GL_LIGHT0 0x4000
#		define GL_LIGHT1 0x4001
#		define GL_LIGHT2 0x4002
#		define GL_LIGHT3 0x4003
#		define GL_LIGHT4 0x4004
#		define GL_LIGHT5 0x4005
#		define GL_LIGHT6 0x4006
#		define GL_LIGHT7 0x4007
#		define GL_AMBIENT 0x1200
#		define GL_DIFFUSE 0x1201
#		define GL_SPECULAR 0x1202
#		define GL_POSITION 0x1203
#		define GL_SPOT_DIRECTION 0x1204
#		define GL_SPOT_EXPONENT 0x1205
#		define GL_SPOT_CUTOFF 0x1206
#		define GL_CONSTANT_ATTENUATION 0x1207
#		define GL_LINEAR_ATTENUATION 0x1208
#		define GL_QUADRATIC_ATTENUATION 0x1209
#		define GL_COMPILE 0x1300
#		define GL_COMPILE_AND_EXECUTE 0x1301
#		define GL_CLEAR 0x1500
#		define GL_AND 0x1501
#		define GL_AND_REVERSE 0x1502
#		define GL_COPY 0x1503
#		define GL_AND_INVERTED 0x1504
#		define GL_NOOP 0x1505
#		define GL_XOR 0x1506
#		define GL_OR 0x1507
#		define GL_NOR 0x1508
#		define GL_EQUIV 0x1509
#		define GL_INVERT 0x150A
#		define GL_OR_REVERSE 0x150B
#		define GL_COPY_INVERTED 0x150C
#		define GL_OR_INVERTED 0x150D
#		define GL_NAND 0x150E
#		define GL_SET 0x150F
#		define GL_EMISSION 0x1600
#		define GL_SHININESS 0x1601
#		define GL_AMBIENT_AND_DIFFUSE 0x1602
#		define GL_COLOR_INDEXES 0x1603
#		define GL_MODELVIEW 0x1700
#		define GL_PROJECTION 0x1701
#		define GL_TEXTURE 0x1702
#		define GL_COLOR 0x1800
#		define GL_DEPTH 0x1801
#		define GL_STENCIL 0x1802
#		define GL_COLOR_INDEX 0x1900
#		define GL_STENCIL_INDEX 0x1901
#		define GL_DEPTH_COMPONENT 0x1902
#		define GL_RED 0x1903
#		define GL_GREEN 0x1904
#		define GL_BLUE 0x1905
#		define GL_ALPHA 0x1906
#		define GL_RGB 0x1907
#		define GL_RGBA 0x1908
#		define GL_LUMINANCE 0x1909
#		define GL_LUMINANCE_ALPHA 0x190A
#		define GL_BITMAP 0x1A00
#		define GL_POINT 0x1B00
#		define GL_LINE 0x1B01
#		define GL_FILL 0x1B02
#		define GL_RENDER 0x1C00
#		define GL_FEEDBACK 0x1C01
#		define GL_SELECT 0x1C02
#		define GL_FLAT 0x1D00
#		define GL_SMOOTH 0x1D01
#		define GL_KEEP 0x1E00
#		define GL_REPLACE 0x1E01
#		define GL_INCR 0x1E02
#		define GL_DECR 0x1E03
#		define GL_VENDOR 0x1F00
#		define GL_RENDERER 0x1F01
#		define GL_VERSION 0x1F02
#		define GL_EXTENSIONS 0x1F03
#		define GL_S 0x2000
#		define GL_T 0x2001
#		define GL_R 0x2002
#		define GL_Q 0x2003
#		define GL_MODULATE 0x2100
#		define GL_DECAL 0x2101
#		define GL_TEXTURE_ENV_MODE 0x2200
#		define GL_TEXTURE_ENV_COLOR 0x2201
#		define GL_TEXTURE_ENV 0x2300
#		define GL_EYE_LINEAR 0x2400
#		define GL_OBJECT_LINEAR 0x2401
#		define GL_SPHERE_MAP 0x2402
#		define GL_TEXTURE_GEN_MODE 0x2500
#		define GL_OBJECT_PLANE 0x2501
#		define GL_EYE_PLANE 0x2502
#		define GL_NEAREST 0x2600
#		define GL_LINEAR 0x2601
#		define GL_NEAREST_MIPMAP_NEAREST 0x2700
#		define GL_LINEAR_MIPMAP_NEAREST 0x2701
#		define GL_NEAREST_MIPMAP_LINEAR 0x2702
#		define GL_LINEAR_MIPMAP_LINEAR 0x2703
#		define GL_TEXTURE_MAG_FILTER 0x2800
#		define GL_TEXTURE_MIN_FILTER 0x2801
#		define GL_TEXTURE_WRAP_S 0x2802
#		define GL_TEXTURE_WRAP_T 0x2803
#		define GL_CLAMP 0x2900
#		define GL_REPEAT 0x2901
#		define GL_CLIENT_PIXEL_STORE_BIT 0x00000001
#		define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#		define GL_CLIENT_ALL_ATTRIB_BITS 0xffffffff
#		define GL_POLYGON_OFFSET_FACTOR 0x8038
#		define GL_POLYGON_OFFSET_UNITS 0x2A00
#		define GL_POLYGON_OFFSET_POINT 0x2A01
#		define GL_POLYGON_OFFSET_LINE 0x2A02
#		define GL_POLYGON_OFFSET_FILL 0x8037
#		define GL_ALPHA4 0x803B
#		define GL_ALPHA8 0x803C
#		define GL_ALPHA12 0x803D
#		define GL_ALPHA16 0x803E
#		define GL_LUMINANCE4 0x803F
#		define GL_LUMINANCE8 0x8040
#		define GL_LUMINANCE12 0x8041
#		define GL_LUMINANCE16 0x8042
#		define GL_LUMINANCE4_ALPHA4 0x8043
#		define GL_LUMINANCE6_ALPHA2 0x8044
#		define GL_LUMINANCE8_ALPHA8 0x8045
#		define GL_LUMINANCE12_ALPHA4 0x8046
#		define GL_LUMINANCE12_ALPHA12 0x8047
#		define GL_LUMINANCE16_ALPHA16 0x8048
#		define GL_INTENSITY 0x8049
#		define GL_INTENSITY4 0x804A
#		define GL_INTENSITY8 0x804B
#		define GL_INTENSITY12 0x804C
#		define GL_INTENSITY16 0x804D
#		define GL_R3_G3_B2 0x2A10
#		define GL_RGB4 0x804F
#		define GL_RGB5 0x8050
#		define GL_RGB8 0x8051
#		define GL_RGB10 0x8052
#		define GL_RGB12 0x8053
#		define GL_RGB16 0x8054
#		define GL_RGBA2 0x8055
#		define GL_RGBA4 0x8056
#		define GL_RGB5_A1 0x8057
#		define GL_RGBA8 0x8058
#		define GL_RGB10_A2 0x8059
#		define GL_RGBA12 0x805A
#		define GL_RGBA16 0x805B
#		define GL_TEXTURE_RED_SIZE 0x805C
#		define GL_TEXTURE_GREEN_SIZE 0x805D
#		define GL_TEXTURE_BLUE_SIZE 0x805E
#		define GL_TEXTURE_ALPHA_SIZE 0x805F
#		define GL_TEXTURE_LUMINANCE_SIZE 0x8060
#		define GL_TEXTURE_INTENSITY_SIZE 0x8061
#		define GL_PROXY_TEXTURE_1D 0x8063
#		define GL_PROXY_TEXTURE_2D 0x8064
#		define GL_TEXTURE_PRIORITY 0x8066
#		define GL_TEXTURE_RESIDENT 0x8067
#		define GL_TEXTURE_BINDING_1D 0x8068
#		define GL_TEXTURE_BINDING_2D 0x8069
#		define GL_VERTEX_ARRAY 0x8074
#		define GL_NORMAL_ARRAY 0x8075
#		define GL_COLOR_ARRAY 0x8076
#		define GL_INDEX_ARRAY 0x8077
#		define GL_TEXTURE_COORD_ARRAY 0x8078
#		define GL_EDGE_FLAG_ARRAY 0x8079
#		define GL_VERTEX_ARRAY_SIZE 0x807A
#		define GL_VERTEX_ARRAY_TYPE 0x807B
#		define GL_VERTEX_ARRAY_STRIDE 0x807C
#		define GL_NORMAL_ARRAY_TYPE 0x807E
#		define GL_NORMAL_ARRAY_STRIDE 0x807F
#		define GL_COLOR_ARRAY_SIZE 0x8081
#		define GL_COLOR_ARRAY_TYPE 0x8082
#		define GL_COLOR_ARRAY_STRIDE 0x8083
#		define GL_INDEX_ARRAY_TYPE 0x8085
#		define GL_INDEX_ARRAY_STRIDE 0x8086
#		define GL_TEXTURE_COORD_ARRAY_SIZE 0x8088
#		define GL_TEXTURE_COORD_ARRAY_TYPE 0x8089
#		define GL_TEXTURE_COORD_ARRAY_STRIDE 0x808A
#		define GL_EDGE_FLAG_ARRAY_STRIDE 0x808C
#		define GL_VERTEX_ARRAY_POINTER 0x808E
#		define GL_NORMAL_ARRAY_POINTER 0x808F
#		define GL_COLOR_ARRAY_POINTER 0x8090
#		define GL_INDEX_ARRAY_POINTER 0x8091
#		define GL_TEXTURE_COORD_ARRAY_POINTER 0x8092
#		define GL_EDGE_FLAG_ARRAY_POINTER 0x8093
#		define GL_V2F 0x2A20
#		define GL_V3F 0x2A21
#		define GL_C4UB_V2F 0x2A22
#		define GL_C4UB_V3F 0x2A23
#		define GL_C3F_V3F 0x2A24
#		define GL_N3F_V3F 0x2A25
#		define GL_C4F_N3F_V3F 0x2A26
#		define GL_T2F_V3F 0x2A27
#		define GL_T4F_V4F 0x2A28
#		define GL_T2F_C4UB_V3F 0x2A29
#		define GL_T2F_C3F_V3F 0x2A2A
#		define GL_T2F_N3F_V3F 0x2A2B
#		define GL_T2F_C4F_N3F_V3F 0x2A2C
#		define GL_T4F_C4F_N3F_V4F 0x2A2D
#		define GL_EXT_vertex_array 1
#		define GL_EXT_bgra 1
#		define GL_EXT_paletted_texture 1
#		define GL_WIN_swap_hint 1
#		define GL_WIN_draw_range_elements 1
#		define GL_WIN_phong_shading 1
#		define GL_WIN_specular_fog 1
#		define GL_VERTEX_ARRAY_EXT 0x8074
#		define GL_NORMAL_ARRAY_EXT 0x8075
#		define GL_COLOR_ARRAY_EXT 0x8076
#		define GL_INDEX_ARRAY_EXT 0x8077
#		define GL_TEXTURE_COORD_ARRAY_EXT 0x8078
#		define GL_EDGE_FLAG_ARRAY_EXT 0x8079
#		define GL_VERTEX_ARRAY_SIZE_EXT 0x807A
#		define GL_VERTEX_ARRAY_TYPE_EXT 0x807B
#		define GL_VERTEX_ARRAY_STRIDE_EXT 0x807C
#		define GL_VERTEX_ARRAY_COUNT_EXT 0x807D
#		define GL_NORMAL_ARRAY_TYPE_EXT 0x807E
#		define GL_NORMAL_ARRAY_STRIDE_EXT 0x807F
#		define GL_NORMAL_ARRAY_COUNT_EXT 0x8080
#		define GL_COLOR_ARRAY_SIZE_EXT 0x8081
#		define GL_COLOR_ARRAY_TYPE_EXT 0x8082
#		define GL_COLOR_ARRAY_STRIDE_EXT 0x8083
#		define GL_COLOR_ARRAY_COUNT_EXT 0x8084
#		define GL_INDEX_ARRAY_TYPE_EXT 0x8085
#		define GL_INDEX_ARRAY_STRIDE_EXT 0x8086
#		define GL_INDEX_ARRAY_COUNT_EXT 0x8087
#		define GL_TEXTURE_COORD_ARRAY_SIZE_EXT 0x8088
#		define GL_TEXTURE_COORD_ARRAY_TYPE_EXT 0x8089
#		define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#		define GL_TEXTURE_COORD_ARRAY_COUNT_EXT 0x808B
#		define GL_EDGE_FLAG_ARRAY_STRIDE_EXT 0x808C
#		define GL_EDGE_FLAG_ARRAY_COUNT_EXT 0x808D
#		define GL_VERTEX_ARRAY_POINTER_EXT 0x808E
#		define GL_NORMAL_ARRAY_POINTER_EXT 0x808F
#		define GL_COLOR_ARRAY_POINTER_EXT 0x8090
#		define GL_INDEX_ARRAY_POINTER_EXT 0x8091
#		define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#		define GL_EDGE_FLAG_ARRAY_POINTER_EXT 0x8093
#		define GL_DOUBLE_EXT GL_DOUBLE
#		define GL_BGR_EXT 0x80E0
#		define GL_BGRA_EXT 0x80E1
#		define GL_COLOR_TABLE_FORMAT_EXT 0x80D8
#		define GL_COLOR_TABLE_WIDTH_EXT 0x80D9
#		define GL_COLOR_TABLE_RED_SIZE_EXT 0x80DA
#		define GL_COLOR_TABLE_GREEN_SIZE_EXT 0x80DB
#		define GL_COLOR_TABLE_BLUE_SIZE_EXT 0x80DC
#		define GL_COLOR_TABLE_ALPHA_SIZE_EXT 0x80DD
#		define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT 0x80DE
#		define GL_COLOR_TABLE_INTENSITY_SIZE_EXT 0x80DF
#		define GL_COLOR_INDEX1_EXT 0x80E2
#		define GL_COLOR_INDEX2_EXT 0x80E3
#		define GL_COLOR_INDEX4_EXT 0x80E4
#		define GL_COLOR_INDEX8_EXT 0x80E5
#		define GL_COLOR_INDEX12_EXT 0x80E6
#		define GL_COLOR_INDEX16_EXT 0x80E7
#		define GL_MAX_ELEMENTS_VERTICES_WIN 0x80E8
#		define GL_MAX_ELEMENTS_INDICES_WIN 0x80E9
#		define GL_PHONG_WIN 0x80EA
#		define GL_PHONG_HINT_WIN 0x80EB
#		define GL_FOG_SPECULAR_TEXTURE_WIN 0x80EC
#		define GL_LOGIC_OP GL_INDEX_LOGIC_OP
#		define GL_TEXTURE_COMPONENTS GL_TEXTURE_INTERNAL_FORMAT

		typedef void (FGL_APIENTRY fgl_func_glAccum)(GLenum op, GLfloat value);
		fgl_api fgl_func_glAccum* fgl_glAccum;
#		define glAccum fgl_glAccum
		typedef void (FGL_APIENTRY fgl_func_glAlphaFunc)(GLenum func, GLclampf ref);
		fgl_api fgl_func_glAlphaFunc* fgl_glAlphaFunc;
#		define glAlphaFunc fgl_glAlphaFunc
		typedef GLboolean (FGL_APIENTRY fgl_func_glAreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences);
		fgl_api fgl_func_glAreTexturesResident* fgl_glAreTexturesResident;
#		define glAreTexturesResident fgl_glAreTexturesResident
		typedef void (FGL_APIENTRY fgl_func_glArrayElement)(GLint i);
		fgl_api fgl_func_glArrayElement* fgl_glArrayElement;
#		define glArrayElement fgl_glArrayElement
		typedef void (FGL_APIENTRY fgl_func_glBegin)(GLenum mode);
		fgl_api fgl_func_glBegin* fgl_glBegin;
#		define glBegin fgl_glBegin
		typedef void (FGL_APIENTRY fgl_func_glBindTexture)(GLenum target, GLuint texture);
		fgl_api fgl_func_glBindTexture* fgl_glBindTexture;
#		define glBindTexture fgl_glBindTexture
		typedef void (FGL_APIENTRY fgl_func_glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
		fgl_api fgl_func_glBitmap* fgl_glBitmap;
#		define glBitmap fgl_glBitmap
		typedef void (FGL_APIENTRY fgl_func_glBlendFunc)(GLenum sfactor, GLenum dfactor);
		fgl_api fgl_func_glBlendFunc* fgl_glBlendFunc;
#		define glBlendFunc fgl_glBlendFunc
		typedef void (FGL_APIENTRY fgl_func_glCallList)(GLuint list);
		fgl_api fgl_func_glCallList* fgl_glCallList;
#		define glCallList fgl_glCallList
		typedef void (FGL_APIENTRY fgl_func_glCallLists)(GLsizei n, GLenum type, const GLvoid *lists);
		fgl_api fgl_func_glCallLists* fgl_glCallLists;
#		define glCallLists fgl_glCallLists
		typedef void (FGL_APIENTRY fgl_func_glClear)(GLbitfield mask);
		fgl_api fgl_func_glClear* fgl_glClear;
#		define glClear fgl_glClear
		typedef void (FGL_APIENTRY fgl_func_glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		fgl_api fgl_func_glClearAccum* fgl_glClearAccum;
#		define glClearAccum fgl_glClearAccum
		typedef void (FGL_APIENTRY fgl_func_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
		fgl_api fgl_func_glClearColor* fgl_glClearColor;
#		define glClearColor fgl_glClearColor
		typedef void (FGL_APIENTRY fgl_func_glClearDepth)(GLclampd depth);
		fgl_api fgl_func_glClearDepth* fgl_glClearDepth;
#		define glClearDepth fgl_glClearDepth
		typedef void (FGL_APIENTRY fgl_func_glClearIndex)(GLfloat c);
		fgl_api fgl_func_glClearIndex* fgl_glClearIndex;
#		define glClearIndex fgl_glClearIndex
		typedef void (FGL_APIENTRY fgl_func_glClearStencil)(GLint s);
		fgl_api fgl_func_glClearStencil* fgl_glClearStencil;
#		define glClearStencil fgl_glClearStencil
		typedef void (FGL_APIENTRY fgl_func_glClipPlane)(GLenum plane, const GLdouble *equation);
		fgl_api fgl_func_glClipPlane* fgl_glClipPlane;
#		define glClipPlane fgl_glClipPlane
		typedef void (FGL_APIENTRY fgl_func_glColor3b)(GLbyte red, GLbyte green, GLbyte blue);
		fgl_api fgl_func_glColor3b* fgl_glColor3b;
#		define glColor3b fgl_glColor3b
		typedef void (FGL_APIENTRY fgl_func_glColor3bv)(const GLbyte *v);
		fgl_api fgl_func_glColor3bv* fgl_glColor3bv;
#		define glColor3bv fgl_glColor3bv
		typedef void (FGL_APIENTRY fgl_func_glColor3d)(GLdouble red, GLdouble green, GLdouble blue);
		fgl_api fgl_func_glColor3d* fgl_glColor3d;
#		define glColor3d fgl_glColor3d
		typedef void (FGL_APIENTRY fgl_func_glColor3dv)(const GLdouble *v);
		fgl_api fgl_func_glColor3dv* fgl_glColor3dv;
#		define glColor3dv fgl_glColor3dv
		typedef void (FGL_APIENTRY fgl_func_glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
		fgl_api fgl_func_glColor3f* fgl_glColor3f;
#		define glColor3f fgl_glColor3f
		typedef void (FGL_APIENTRY fgl_func_glColor3fv)(const GLfloat *v);
		fgl_api fgl_func_glColor3fv* fgl_glColor3fv;
#		define glColor3fv fgl_glColor3fv
		typedef void (FGL_APIENTRY fgl_func_glColor3i)(GLint red, GLint green, GLint blue);
		fgl_api fgl_func_glColor3i* fgl_glColor3i;
#		define glColor3i fgl_glColor3i
		typedef void (FGL_APIENTRY fgl_func_glColor3iv)(const GLint *v);
		fgl_api fgl_func_glColor3iv* fgl_glColor3iv;
#		define glColor3iv fgl_glColor3iv
		typedef void (FGL_APIENTRY fgl_func_glColor3s)(GLshort red, GLshort green, GLshort blue);
		fgl_api fgl_func_glColor3s* fgl_glColor3s;
#		define glColor3s fgl_glColor3s
		typedef void (FGL_APIENTRY fgl_func_glColor3sv)(const GLshort *v);
		fgl_api fgl_func_glColor3sv* fgl_glColor3sv;
#		define glColor3sv fgl_glColor3sv
		typedef void (FGL_APIENTRY fgl_func_glColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
		fgl_api fgl_func_glColor3ub* fgl_glColor3ub;
#		define glColor3ub fgl_glColor3ub
		typedef void (FGL_APIENTRY fgl_func_glColor3ubv)(const GLubyte *v);
		fgl_api fgl_func_glColor3ubv* fgl_glColor3ubv;
#		define glColor3ubv fgl_glColor3ubv
		typedef void (FGL_APIENTRY fgl_func_glColor3ui)(GLuint red, GLuint green, GLuint blue);
		fgl_api fgl_func_glColor3ui* fgl_glColor3ui;
#		define glColor3ui fgl_glColor3ui
		typedef void (FGL_APIENTRY fgl_func_glColor3uiv)(const GLuint *v);
		fgl_api fgl_func_glColor3uiv* fgl_glColor3uiv;
#		define glColor3uiv fgl_glColor3uiv
		typedef void (FGL_APIENTRY fgl_func_glColor3us)(GLushort red, GLushort green, GLushort blue);
		fgl_api fgl_func_glColor3us* fgl_glColor3us;
#		define glColor3us fgl_glColor3us
		typedef void (FGL_APIENTRY fgl_func_glColor3usv)(const GLushort *v);
		fgl_api fgl_func_glColor3usv* fgl_glColor3usv;
#		define glColor3usv fgl_glColor3usv
		typedef void (FGL_APIENTRY fgl_func_glColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
		fgl_api fgl_func_glColor4b* fgl_glColor4b;
#		define glColor4b fgl_glColor4b
		typedef void (FGL_APIENTRY fgl_func_glColor4bv)(const GLbyte *v);
		fgl_api fgl_func_glColor4bv* fgl_glColor4bv;
#		define glColor4bv fgl_glColor4bv
		typedef void (FGL_APIENTRY fgl_func_glColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
		fgl_api fgl_func_glColor4d* fgl_glColor4d;
#		define glColor4d fgl_glColor4d
		typedef void (FGL_APIENTRY fgl_func_glColor4dv)(const GLdouble *v);
		fgl_api fgl_func_glColor4dv* fgl_glColor4dv;
#		define glColor4dv fgl_glColor4dv
		typedef void (FGL_APIENTRY fgl_func_glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		fgl_api fgl_func_glColor4f* fgl_glColor4f;
#		define glColor4f fgl_glColor4f
		typedef void (FGL_APIENTRY fgl_func_glColor4fv)(const GLfloat *v);
		fgl_api fgl_func_glColor4fv* fgl_glColor4fv;
#		define glColor4fv fgl_glColor4fv
		typedef void (FGL_APIENTRY fgl_func_glColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
		fgl_api fgl_func_glColor4i* fgl_glColor4i;
#		define glColor4i fgl_glColor4i
		typedef void (FGL_APIENTRY fgl_func_glColor4iv)(const GLint *v);
		fgl_api fgl_func_glColor4iv* fgl_glColor4iv;
#		define glColor4iv fgl_glColor4iv
		typedef void (FGL_APIENTRY fgl_func_glColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
		fgl_api fgl_func_glColor4s* fgl_glColor4s;
#		define glColor4s fgl_glColor4s
		typedef void (FGL_APIENTRY fgl_func_glColor4sv)(const GLshort *v);
		fgl_api fgl_func_glColor4sv* fgl_glColor4sv;
#		define glColor4sv fgl_glColor4sv
		typedef void (FGL_APIENTRY fgl_func_glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
		fgl_api fgl_func_glColor4ub* fgl_glColor4ub;
#		define glColor4ub fgl_glColor4ub
		typedef void (FGL_APIENTRY fgl_func_glColor4ubv)(const GLubyte *v);
		fgl_api fgl_func_glColor4ubv* fgl_glColor4ubv;
#		define glColor4ubv fgl_glColor4ubv
		typedef void (FGL_APIENTRY fgl_func_glColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
		fgl_api fgl_func_glColor4ui* fgl_glColor4ui;
#		define glColor4ui fgl_glColor4ui
		typedef void (FGL_APIENTRY fgl_func_glColor4uiv)(const GLuint *v);
		fgl_api fgl_func_glColor4uiv* fgl_glColor4uiv;
#		define glColor4uiv fgl_glColor4uiv
		typedef void (FGL_APIENTRY fgl_func_glColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
		fgl_api fgl_func_glColor4us* fgl_glColor4us;
#		define glColor4us fgl_glColor4us
		typedef void (FGL_APIENTRY fgl_func_glColor4usv)(const GLushort *v);
		fgl_api fgl_func_glColor4usv* fgl_glColor4usv;
#		define glColor4usv fgl_glColor4usv
		typedef void (FGL_APIENTRY fgl_func_glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
		fgl_api fgl_func_glColorMask* fgl_glColorMask;
#		define glColorMask fgl_glColorMask
		typedef void (FGL_APIENTRY fgl_func_glColorMaterial)(GLenum face, GLenum mode);
		fgl_api fgl_func_glColorMaterial* fgl_glColorMaterial;
#		define glColorMaterial fgl_glColorMaterial
		typedef void (FGL_APIENTRY fgl_func_glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		fgl_api fgl_func_glColorPointer* fgl_glColorPointer;
#		define glColorPointer fgl_glColorPointer
		typedef void (FGL_APIENTRY fgl_func_glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
		fgl_api fgl_func_glCopyPixels* fgl_glCopyPixels;
#		define glCopyPixels fgl_glCopyPixels
		typedef void (FGL_APIENTRY fgl_func_glCopyTexImage1D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
		fgl_api fgl_func_glCopyTexImage1D* fgl_glCopyTexImage1D;
#		define glCopyTexImage1D fgl_glCopyTexImage1D
		typedef void (FGL_APIENTRY fgl_func_glCopyTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
		fgl_api fgl_func_glCopyTexImage2D* fgl_glCopyTexImage2D;
#		define glCopyTexImage2D fgl_glCopyTexImage2D
		typedef void (FGL_APIENTRY fgl_func_glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		fgl_api fgl_func_glCopyTexSubImage1D* fgl_glCopyTexSubImage1D;
#		define glCopyTexSubImage1D fgl_glCopyTexSubImage1D
		typedef void (FGL_APIENTRY fgl_func_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glCopyTexSubImage2D* fgl_glCopyTexSubImage2D;
#		define glCopyTexSubImage2D fgl_glCopyTexSubImage2D
		typedef void (FGL_APIENTRY fgl_func_glCullFace)(GLenum mode);
		fgl_api fgl_func_glCullFace* fgl_glCullFace;
#		define glCullFace fgl_glCullFace
		typedef void (FGL_APIENTRY fgl_func_glDeleteLists)(GLuint list, GLsizei range);
		fgl_api fgl_func_glDeleteLists* fgl_glDeleteLists;
#		define glDeleteLists fgl_glDeleteLists
		typedef void (FGL_APIENTRY fgl_func_glDeleteTextures)(GLsizei n, const GLuint *textures);
		fgl_api fgl_func_glDeleteTextures* fgl_glDeleteTextures;
#		define glDeleteTextures fgl_glDeleteTextures
		typedef void (FGL_APIENTRY fgl_func_glDepthFunc)(GLenum func);
		fgl_api fgl_func_glDepthFunc* fgl_glDepthFunc;
#		define glDepthFunc fgl_glDepthFunc
		typedef void (FGL_APIENTRY fgl_func_glDepthMask)(GLboolean flag);
		fgl_api fgl_func_glDepthMask* fgl_glDepthMask;
#		define glDepthMask fgl_glDepthMask
		typedef void (FGL_APIENTRY fgl_func_glDepthRange)(GLclampd zNear, GLclampd zFar);
		fgl_api fgl_func_glDepthRange* fgl_glDepthRange;
#		define glDepthRange fgl_glDepthRange
		typedef void (FGL_APIENTRY fgl_func_glDisable)(GLenum cap);
		fgl_api fgl_func_glDisable* fgl_glDisable;
#		define glDisable fgl_glDisable
		typedef void (FGL_APIENTRY fgl_func_glDisableClientState)(GLenum array);
		fgl_api fgl_func_glDisableClientState* fgl_glDisableClientState;
#		define glDisableClientState fgl_glDisableClientState
		typedef void (FGL_APIENTRY fgl_func_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
		fgl_api fgl_func_glDrawArrays* fgl_glDrawArrays;
#		define glDrawArrays fgl_glDrawArrays
		typedef void (FGL_APIENTRY fgl_func_glDrawBuffer)(GLenum mode);
		fgl_api fgl_func_glDrawBuffer* fgl_glDrawBuffer;
#		define glDrawBuffer fgl_glDrawBuffer
		typedef void (FGL_APIENTRY fgl_func_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
		fgl_api fgl_func_glDrawElements* fgl_glDrawElements;
#		define glDrawElements fgl_glDrawElements
		typedef void (FGL_APIENTRY fgl_func_glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
		fgl_api fgl_func_glDrawPixels* fgl_glDrawPixels;
#		define glDrawPixels fgl_glDrawPixels
		typedef void (FGL_APIENTRY fgl_func_glEdgeFlag)(GLboolean flag);
		fgl_api fgl_func_glEdgeFlag* fgl_glEdgeFlag;
#		define glEdgeFlag fgl_glEdgeFlag
		typedef void (FGL_APIENTRY fgl_func_glEdgeFlagPointer)(GLsizei stride, const GLvoid *pointer);
		fgl_api fgl_func_glEdgeFlagPointer* fgl_glEdgeFlagPointer;
#		define glEdgeFlagPointer fgl_glEdgeFlagPointer
		typedef void (FGL_APIENTRY fgl_func_glEdgeFlagv)(const GLboolean *flag);
		fgl_api fgl_func_glEdgeFlagv* fgl_glEdgeFlagv;
#		define glEdgeFlagv fgl_glEdgeFlagv
		typedef void (FGL_APIENTRY fgl_func_glEnable)(GLenum cap);
		fgl_api fgl_func_glEnable* fgl_glEnable;
#		define glEnable fgl_glEnable
		typedef void (FGL_APIENTRY fgl_func_glEnableClientState)(GLenum array);
		fgl_api fgl_func_glEnableClientState* fgl_glEnableClientState;
#		define glEnableClientState fgl_glEnableClientState
		typedef void (FGL_APIENTRY fgl_func_glEnd)(void);
		fgl_api fgl_func_glEnd* fgl_glEnd;
#		define glEnd fgl_glEnd
		typedef void (FGL_APIENTRY fgl_func_glEndList)(void);
		fgl_api fgl_func_glEndList* fgl_glEndList;
#		define glEndList fgl_glEndList
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord1d)(GLdouble u);
		fgl_api fgl_func_glEvalCoord1d* fgl_glEvalCoord1d;
#		define glEvalCoord1d fgl_glEvalCoord1d
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord1dv)(const GLdouble *u);
		fgl_api fgl_func_glEvalCoord1dv* fgl_glEvalCoord1dv;
#		define glEvalCoord1dv fgl_glEvalCoord1dv
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord1f)(GLfloat u);
		fgl_api fgl_func_glEvalCoord1f* fgl_glEvalCoord1f;
#		define glEvalCoord1f fgl_glEvalCoord1f
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord1fv)(const GLfloat *u);
		fgl_api fgl_func_glEvalCoord1fv* fgl_glEvalCoord1fv;
#		define glEvalCoord1fv fgl_glEvalCoord1fv
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord2d)(GLdouble u, GLdouble v);
		fgl_api fgl_func_glEvalCoord2d* fgl_glEvalCoord2d;
#		define glEvalCoord2d fgl_glEvalCoord2d
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord2dv)(const GLdouble *u);
		fgl_api fgl_func_glEvalCoord2dv* fgl_glEvalCoord2dv;
#		define glEvalCoord2dv fgl_glEvalCoord2dv
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord2f)(GLfloat u, GLfloat v);
		fgl_api fgl_func_glEvalCoord2f* fgl_glEvalCoord2f;
#		define glEvalCoord2f fgl_glEvalCoord2f
		typedef void (FGL_APIENTRY fgl_func_glEvalCoord2fv)(const GLfloat *u);
		fgl_api fgl_func_glEvalCoord2fv* fgl_glEvalCoord2fv;
#		define glEvalCoord2fv fgl_glEvalCoord2fv
		typedef void (FGL_APIENTRY fgl_func_glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
		fgl_api fgl_func_glEvalMesh1* fgl_glEvalMesh1;
#		define glEvalMesh1 fgl_glEvalMesh1
		typedef void (FGL_APIENTRY fgl_func_glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
		fgl_api fgl_func_glEvalMesh2* fgl_glEvalMesh2;
#		define glEvalMesh2 fgl_glEvalMesh2
		typedef void (FGL_APIENTRY fgl_func_glEvalPoint1)(GLint i);
		fgl_api fgl_func_glEvalPoint1* fgl_glEvalPoint1;
#		define glEvalPoint1 fgl_glEvalPoint1
		typedef void (FGL_APIENTRY fgl_func_glEvalPoint2)(GLint i, GLint j);
		fgl_api fgl_func_glEvalPoint2* fgl_glEvalPoint2;
#		define glEvalPoint2 fgl_glEvalPoint2
		typedef void (FGL_APIENTRY fgl_func_glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
		fgl_api fgl_func_glFeedbackBuffer* fgl_glFeedbackBuffer;
#		define glFeedbackBuffer fgl_glFeedbackBuffer
		typedef void (FGL_APIENTRY fgl_func_glFinish)(void);
		fgl_api fgl_func_glFinish* fgl_glFinish;
#		define glFinish fgl_glFinish
		typedef void (FGL_APIENTRY fgl_func_glFlush)(void);
		fgl_api fgl_func_glFlush* fgl_glFlush;
#		define glFlush fgl_glFlush
		typedef void (FGL_APIENTRY fgl_func_glFogf)(GLenum pname, GLfloat param);
		fgl_api fgl_func_glFogf* fgl_glFogf;
#		define glFogf fgl_glFogf
		typedef void (FGL_APIENTRY fgl_func_glFogfv)(GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glFogfv* fgl_glFogfv;
#		define glFogfv fgl_glFogfv
		typedef void (FGL_APIENTRY fgl_func_glFogi)(GLenum pname, GLint param);
		fgl_api fgl_func_glFogi* fgl_glFogi;
#		define glFogi fgl_glFogi
		typedef void (FGL_APIENTRY fgl_func_glFogiv)(GLenum pname, const GLint *params);
		fgl_api fgl_func_glFogiv* fgl_glFogiv;
#		define glFogiv fgl_glFogiv
		typedef void (FGL_APIENTRY fgl_func_glFrontFace)(GLenum mode);
		fgl_api fgl_func_glFrontFace* fgl_glFrontFace;
#		define glFrontFace fgl_glFrontFace
		typedef void (FGL_APIENTRY fgl_func_glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		fgl_api fgl_func_glFrustum* fgl_glFrustum;
#		define glFrustum fgl_glFrustum
		typedef GLuint (FGL_APIENTRY fgl_func_glGenLists)(GLsizei range);
		fgl_api fgl_func_glGenLists* fgl_glGenLists;
#		define glGenLists fgl_glGenLists
		typedef void (FGL_APIENTRY fgl_func_glGenTextures)(GLsizei n, GLuint *textures);
		fgl_api fgl_func_glGenTextures* fgl_glGenTextures;
#		define glGenTextures fgl_glGenTextures
		typedef void (FGL_APIENTRY fgl_func_glGetBooleanv)(GLenum pname, GLboolean *params);
		fgl_api fgl_func_glGetBooleanv* fgl_glGetBooleanv;
#		define glGetBooleanv fgl_glGetBooleanv
		typedef void (FGL_APIENTRY fgl_func_glGetClipPlane)(GLenum plane, GLdouble *equation);
		fgl_api fgl_func_glGetClipPlane* fgl_glGetClipPlane;
#		define glGetClipPlane fgl_glGetClipPlane
		typedef void (FGL_APIENTRY fgl_func_glGetDoublev)(GLenum pname, GLdouble *params);
		fgl_api fgl_func_glGetDoublev* fgl_glGetDoublev;
#		define glGetDoublev fgl_glGetDoublev
		typedef GLenum (FGL_APIENTRY fgl_func_glGetError)(void);
		fgl_api fgl_func_glGetError* fgl_glGetError;
#		define glGetError fgl_glGetError
		typedef void (FGL_APIENTRY fgl_func_glGetFloatv)(GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetFloatv* fgl_glGetFloatv;
#		define glGetFloatv fgl_glGetFloatv
		typedef void (FGL_APIENTRY fgl_func_glGetIntegerv)(GLenum pname, GLint *params);
		fgl_api fgl_func_glGetIntegerv* fgl_glGetIntegerv;
#		define glGetIntegerv fgl_glGetIntegerv
		typedef void (FGL_APIENTRY fgl_func_glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetLightfv* fgl_glGetLightfv;
#		define glGetLightfv fgl_glGetLightfv
		typedef void (FGL_APIENTRY fgl_func_glGetLightiv)(GLenum light, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetLightiv* fgl_glGetLightiv;
#		define glGetLightiv fgl_glGetLightiv
		typedef void (FGL_APIENTRY fgl_func_glGetMapdv)(GLenum target, GLenum query, GLdouble *v);
		fgl_api fgl_func_glGetMapdv* fgl_glGetMapdv;
#		define glGetMapdv fgl_glGetMapdv
		typedef void (FGL_APIENTRY fgl_func_glGetMapfv)(GLenum target, GLenum query, GLfloat *v);
		fgl_api fgl_func_glGetMapfv* fgl_glGetMapfv;
#		define glGetMapfv fgl_glGetMapfv
		typedef void (FGL_APIENTRY fgl_func_glGetMapiv)(GLenum target, GLenum query, GLint *v);
		fgl_api fgl_func_glGetMapiv* fgl_glGetMapiv;
#		define glGetMapiv fgl_glGetMapiv
		typedef void (FGL_APIENTRY fgl_func_glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetMaterialfv* fgl_glGetMaterialfv;
#		define glGetMaterialfv fgl_glGetMaterialfv
		typedef void (FGL_APIENTRY fgl_func_glGetMaterialiv)(GLenum face, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetMaterialiv* fgl_glGetMaterialiv;
#		define glGetMaterialiv fgl_glGetMaterialiv
		typedef void (FGL_APIENTRY fgl_func_glGetPixelMapfv)(GLenum map, GLfloat *values);
		fgl_api fgl_func_glGetPixelMapfv* fgl_glGetPixelMapfv;
#		define glGetPixelMapfv fgl_glGetPixelMapfv
		typedef void (FGL_APIENTRY fgl_func_glGetPixelMapuiv)(GLenum map, GLuint *values);
		fgl_api fgl_func_glGetPixelMapuiv* fgl_glGetPixelMapuiv;
#		define glGetPixelMapuiv fgl_glGetPixelMapuiv
		typedef void (FGL_APIENTRY fgl_func_glGetPixelMapusv)(GLenum map, GLushort *values);
		fgl_api fgl_func_glGetPixelMapusv* fgl_glGetPixelMapusv;
#		define glGetPixelMapusv fgl_glGetPixelMapusv
		typedef void (FGL_APIENTRY fgl_func_glGetPointerv)(GLenum pname, GLvoid* *params);
		fgl_api fgl_func_glGetPointerv* fgl_glGetPointerv;
#		define glGetPointerv fgl_glGetPointerv
		typedef void (FGL_APIENTRY fgl_func_glGetPolygonStipple)(GLubyte *mask);
		fgl_api fgl_func_glGetPolygonStipple* fgl_glGetPolygonStipple;
#		define glGetPolygonStipple fgl_glGetPolygonStipple
		typedef const GLubyte * (FGL_APIENTRY fgl_func_glGetString)(GLenum name);
		fgl_api fgl_func_glGetString* fgl_glGetString;
#		define glGetString fgl_glGetString
		typedef void (FGL_APIENTRY fgl_func_glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetTexEnvfv* fgl_glGetTexEnvfv;
#		define glGetTexEnvfv fgl_glGetTexEnvfv
		typedef void (FGL_APIENTRY fgl_func_glGetTexEnviv)(GLenum target, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTexEnviv* fgl_glGetTexEnviv;
#		define glGetTexEnviv fgl_glGetTexEnviv
		typedef void (FGL_APIENTRY fgl_func_glGetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
		fgl_api fgl_func_glGetTexGendv* fgl_glGetTexGendv;
#		define glGetTexGendv fgl_glGetTexGendv
		typedef void (FGL_APIENTRY fgl_func_glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetTexGenfv* fgl_glGetTexGenfv;
#		define glGetTexGenfv fgl_glGetTexGenfv
		typedef void (FGL_APIENTRY fgl_func_glGetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTexGeniv* fgl_glGetTexGeniv;
#		define glGetTexGeniv fgl_glGetTexGeniv
		typedef void (FGL_APIENTRY fgl_func_glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
		fgl_api fgl_func_glGetTexImage* fgl_glGetTexImage;
#		define glGetTexImage fgl_glGetTexImage
		typedef void (FGL_APIENTRY fgl_func_glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetTexLevelParameterfv* fgl_glGetTexLevelParameterfv;
#		define glGetTexLevelParameterfv fgl_glGetTexLevelParameterfv
		typedef void (FGL_APIENTRY fgl_func_glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTexLevelParameteriv* fgl_glGetTexLevelParameteriv;
#		define glGetTexLevelParameteriv fgl_glGetTexLevelParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetTexParameterfv* fgl_glGetTexParameterfv;
#		define glGetTexParameterfv fgl_glGetTexParameterfv
		typedef void (FGL_APIENTRY fgl_func_glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTexParameteriv* fgl_glGetTexParameteriv;
#		define glGetTexParameteriv fgl_glGetTexParameteriv
		typedef void (FGL_APIENTRY fgl_func_glHint)(GLenum target, GLenum mode);
		fgl_api fgl_func_glHint* fgl_glHint;
#		define glHint fgl_glHint
		typedef void (FGL_APIENTRY fgl_func_glIndexMask)(GLuint mask);
		fgl_api fgl_func_glIndexMask* fgl_glIndexMask;
#		define glIndexMask fgl_glIndexMask
		typedef void (FGL_APIENTRY fgl_func_glIndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
		fgl_api fgl_func_glIndexPointer* fgl_glIndexPointer;
#		define glIndexPointer fgl_glIndexPointer
		typedef void (FGL_APIENTRY fgl_func_glIndexd)(GLdouble c);
		fgl_api fgl_func_glIndexd* fgl_glIndexd;
#		define glIndexd fgl_glIndexd
		typedef void (FGL_APIENTRY fgl_func_glIndexdv)(const GLdouble *c);
		fgl_api fgl_func_glIndexdv* fgl_glIndexdv;
#		define glIndexdv fgl_glIndexdv
		typedef void (FGL_APIENTRY fgl_func_glIndexf)(GLfloat c);
		fgl_api fgl_func_glIndexf* fgl_glIndexf;
#		define glIndexf fgl_glIndexf
		typedef void (FGL_APIENTRY fgl_func_glIndexfv)(const GLfloat *c);
		fgl_api fgl_func_glIndexfv* fgl_glIndexfv;
#		define glIndexfv fgl_glIndexfv
		typedef void (FGL_APIENTRY fgl_func_glIndexi)(GLint c);
		fgl_api fgl_func_glIndexi* fgl_glIndexi;
#		define glIndexi fgl_glIndexi
		typedef void (FGL_APIENTRY fgl_func_glIndexiv)(const GLint *c);
		fgl_api fgl_func_glIndexiv* fgl_glIndexiv;
#		define glIndexiv fgl_glIndexiv
		typedef void (FGL_APIENTRY fgl_func_glIndexs)(GLshort c);
		fgl_api fgl_func_glIndexs* fgl_glIndexs;
#		define glIndexs fgl_glIndexs
		typedef void (FGL_APIENTRY fgl_func_glIndexsv)(const GLshort *c);
		fgl_api fgl_func_glIndexsv* fgl_glIndexsv;
#		define glIndexsv fgl_glIndexsv
		typedef void (FGL_APIENTRY fgl_func_glIndexub)(GLubyte c);
		fgl_api fgl_func_glIndexub* fgl_glIndexub;
#		define glIndexub fgl_glIndexub
		typedef void (FGL_APIENTRY fgl_func_glIndexubv)(const GLubyte *c);
		fgl_api fgl_func_glIndexubv* fgl_glIndexubv;
#		define glIndexubv fgl_glIndexubv
		typedef void (FGL_APIENTRY fgl_func_glInitNames)(void);
		fgl_api fgl_func_glInitNames* fgl_glInitNames;
#		define glInitNames fgl_glInitNames
		typedef void (FGL_APIENTRY fgl_func_glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);
		fgl_api fgl_func_glInterleavedArrays* fgl_glInterleavedArrays;
#		define glInterleavedArrays fgl_glInterleavedArrays
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsEnabled)(GLenum cap);
		fgl_api fgl_func_glIsEnabled* fgl_glIsEnabled;
#		define glIsEnabled fgl_glIsEnabled
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsList)(GLuint list);
		fgl_api fgl_func_glIsList* fgl_glIsList;
#		define glIsList fgl_glIsList
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsTexture)(GLuint texture);
		fgl_api fgl_func_glIsTexture* fgl_glIsTexture;
#		define glIsTexture fgl_glIsTexture
		typedef void (FGL_APIENTRY fgl_func_glLightModelf)(GLenum pname, GLfloat param);
		fgl_api fgl_func_glLightModelf* fgl_glLightModelf;
#		define glLightModelf fgl_glLightModelf
		typedef void (FGL_APIENTRY fgl_func_glLightModelfv)(GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glLightModelfv* fgl_glLightModelfv;
#		define glLightModelfv fgl_glLightModelfv
		typedef void (FGL_APIENTRY fgl_func_glLightModeli)(GLenum pname, GLint param);
		fgl_api fgl_func_glLightModeli* fgl_glLightModeli;
#		define glLightModeli fgl_glLightModeli
		typedef void (FGL_APIENTRY fgl_func_glLightModeliv)(GLenum pname, const GLint *params);
		fgl_api fgl_func_glLightModeliv* fgl_glLightModeliv;
#		define glLightModeliv fgl_glLightModeliv
		typedef void (FGL_APIENTRY fgl_func_glLightf)(GLenum light, GLenum pname, GLfloat param);
		fgl_api fgl_func_glLightf* fgl_glLightf;
#		define glLightf fgl_glLightf
		typedef void (FGL_APIENTRY fgl_func_glLightfv)(GLenum light, GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glLightfv* fgl_glLightfv;
#		define glLightfv fgl_glLightfv
		typedef void (FGL_APIENTRY fgl_func_glLighti)(GLenum light, GLenum pname, GLint param);
		fgl_api fgl_func_glLighti* fgl_glLighti;
#		define glLighti fgl_glLighti
		typedef void (FGL_APIENTRY fgl_func_glLightiv)(GLenum light, GLenum pname, const GLint *params);
		fgl_api fgl_func_glLightiv* fgl_glLightiv;
#		define glLightiv fgl_glLightiv
		typedef void (FGL_APIENTRY fgl_func_glLineStipple)(GLint factor, GLushort pattern);
		fgl_api fgl_func_glLineStipple* fgl_glLineStipple;
#		define glLineStipple fgl_glLineStipple
		typedef void (FGL_APIENTRY fgl_func_glLineWidth)(GLfloat width);
		fgl_api fgl_func_glLineWidth* fgl_glLineWidth;
#		define glLineWidth fgl_glLineWidth
		typedef void (FGL_APIENTRY fgl_func_glListBase)(GLuint base);
		fgl_api fgl_func_glListBase* fgl_glListBase;
#		define glListBase fgl_glListBase
		typedef void (FGL_APIENTRY fgl_func_glLoadIdentity)(void);
		fgl_api fgl_func_glLoadIdentity* fgl_glLoadIdentity;
#		define glLoadIdentity fgl_glLoadIdentity
		typedef void (FGL_APIENTRY fgl_func_glLoadMatrixd)(const GLdouble *m);
		fgl_api fgl_func_glLoadMatrixd* fgl_glLoadMatrixd;
#		define glLoadMatrixd fgl_glLoadMatrixd
		typedef void (FGL_APIENTRY fgl_func_glLoadMatrixf)(const GLfloat *m);
		fgl_api fgl_func_glLoadMatrixf* fgl_glLoadMatrixf;
#		define glLoadMatrixf fgl_glLoadMatrixf
		typedef void (FGL_APIENTRY fgl_func_glLoadName)(GLuint name);
		fgl_api fgl_func_glLoadName* fgl_glLoadName;
#		define glLoadName fgl_glLoadName
		typedef void (FGL_APIENTRY fgl_func_glLogicOp)(GLenum opcode);
		fgl_api fgl_func_glLogicOp* fgl_glLogicOp;
#		define glLogicOp fgl_glLogicOp
		typedef void (FGL_APIENTRY fgl_func_glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
		fgl_api fgl_func_glMap1d* fgl_glMap1d;
#		define glMap1d fgl_glMap1d
		typedef void (FGL_APIENTRY fgl_func_glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
		fgl_api fgl_func_glMap1f* fgl_glMap1f;
#		define glMap1f fgl_glMap1f
		typedef void (FGL_APIENTRY fgl_func_glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
		fgl_api fgl_func_glMap2d* fgl_glMap2d;
#		define glMap2d fgl_glMap2d
		typedef void (FGL_APIENTRY fgl_func_glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
		fgl_api fgl_func_glMap2f* fgl_glMap2f;
#		define glMap2f fgl_glMap2f
		typedef void (FGL_APIENTRY fgl_func_glMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
		fgl_api fgl_func_glMapGrid1d* fgl_glMapGrid1d;
#		define glMapGrid1d fgl_glMapGrid1d
		typedef void (FGL_APIENTRY fgl_func_glMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
		fgl_api fgl_func_glMapGrid1f* fgl_glMapGrid1f;
#		define glMapGrid1f fgl_glMapGrid1f
		typedef void (FGL_APIENTRY fgl_func_glMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
		fgl_api fgl_func_glMapGrid2d* fgl_glMapGrid2d;
#		define glMapGrid2d fgl_glMapGrid2d
		typedef void (FGL_APIENTRY fgl_func_glMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
		fgl_api fgl_func_glMapGrid2f* fgl_glMapGrid2f;
#		define glMapGrid2f fgl_glMapGrid2f
		typedef void (FGL_APIENTRY fgl_func_glMaterialf)(GLenum face, GLenum pname, GLfloat param);
		fgl_api fgl_func_glMaterialf* fgl_glMaterialf;
#		define glMaterialf fgl_glMaterialf
		typedef void (FGL_APIENTRY fgl_func_glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glMaterialfv* fgl_glMaterialfv;
#		define glMaterialfv fgl_glMaterialfv
		typedef void (FGL_APIENTRY fgl_func_glMateriali)(GLenum face, GLenum pname, GLint param);
		fgl_api fgl_func_glMateriali* fgl_glMateriali;
#		define glMateriali fgl_glMateriali
		typedef void (FGL_APIENTRY fgl_func_glMaterialiv)(GLenum face, GLenum pname, const GLint *params);
		fgl_api fgl_func_glMaterialiv* fgl_glMaterialiv;
#		define glMaterialiv fgl_glMaterialiv
		typedef void (FGL_APIENTRY fgl_func_glMatrixMode)(GLenum mode);
		fgl_api fgl_func_glMatrixMode* fgl_glMatrixMode;
#		define glMatrixMode fgl_glMatrixMode
		typedef void (FGL_APIENTRY fgl_func_glMultMatrixd)(const GLdouble *m);
		fgl_api fgl_func_glMultMatrixd* fgl_glMultMatrixd;
#		define glMultMatrixd fgl_glMultMatrixd
		typedef void (FGL_APIENTRY fgl_func_glMultMatrixf)(const GLfloat *m);
		fgl_api fgl_func_glMultMatrixf* fgl_glMultMatrixf;
#		define glMultMatrixf fgl_glMultMatrixf
		typedef void (FGL_APIENTRY fgl_func_glNewList)(GLuint list, GLenum mode);
		fgl_api fgl_func_glNewList* fgl_glNewList;
#		define glNewList fgl_glNewList
		typedef void (FGL_APIENTRY fgl_func_glNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
		fgl_api fgl_func_glNormal3b* fgl_glNormal3b;
#		define glNormal3b fgl_glNormal3b
		typedef void (FGL_APIENTRY fgl_func_glNormal3bv)(const GLbyte *v);
		fgl_api fgl_func_glNormal3bv* fgl_glNormal3bv;
#		define glNormal3bv fgl_glNormal3bv
		typedef void (FGL_APIENTRY fgl_func_glNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
		fgl_api fgl_func_glNormal3d* fgl_glNormal3d;
#		define glNormal3d fgl_glNormal3d
		typedef void (FGL_APIENTRY fgl_func_glNormal3dv)(const GLdouble *v);
		fgl_api fgl_func_glNormal3dv* fgl_glNormal3dv;
#		define glNormal3dv fgl_glNormal3dv
		typedef void (FGL_APIENTRY fgl_func_glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
		fgl_api fgl_func_glNormal3f* fgl_glNormal3f;
#		define glNormal3f fgl_glNormal3f
		typedef void (FGL_APIENTRY fgl_func_glNormal3fv)(const GLfloat *v);
		fgl_api fgl_func_glNormal3fv* fgl_glNormal3fv;
#		define glNormal3fv fgl_glNormal3fv
		typedef void (FGL_APIENTRY fgl_func_glNormal3i)(GLint nx, GLint ny, GLint nz);
		fgl_api fgl_func_glNormal3i* fgl_glNormal3i;
#		define glNormal3i fgl_glNormal3i
		typedef void (FGL_APIENTRY fgl_func_glNormal3iv)(const GLint *v);
		fgl_api fgl_func_glNormal3iv* fgl_glNormal3iv;
#		define glNormal3iv fgl_glNormal3iv
		typedef void (FGL_APIENTRY fgl_func_glNormal3s)(GLshort nx, GLshort ny, GLshort nz);
		fgl_api fgl_func_glNormal3s* fgl_glNormal3s;
#		define glNormal3s fgl_glNormal3s
		typedef void (FGL_APIENTRY fgl_func_glNormal3sv)(const GLshort *v);
		fgl_api fgl_func_glNormal3sv* fgl_glNormal3sv;
#		define glNormal3sv fgl_glNormal3sv
		typedef void (FGL_APIENTRY fgl_func_glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
		fgl_api fgl_func_glNormalPointer* fgl_glNormalPointer;
#		define glNormalPointer fgl_glNormalPointer
		typedef void (FGL_APIENTRY fgl_func_glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		fgl_api fgl_func_glOrtho* fgl_glOrtho;
#		define glOrtho fgl_glOrtho
		typedef void (FGL_APIENTRY fgl_func_glPassThrough)(GLfloat token);
		fgl_api fgl_func_glPassThrough* fgl_glPassThrough;
#		define glPassThrough fgl_glPassThrough
		typedef void (FGL_APIENTRY fgl_func_glPixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat *values);
		fgl_api fgl_func_glPixelMapfv* fgl_glPixelMapfv;
#		define glPixelMapfv fgl_glPixelMapfv
		typedef void (FGL_APIENTRY fgl_func_glPixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint *values);
		fgl_api fgl_func_glPixelMapuiv* fgl_glPixelMapuiv;
#		define glPixelMapuiv fgl_glPixelMapuiv
		typedef void (FGL_APIENTRY fgl_func_glPixelMapusv)(GLenum map, GLsizei mapsize, const GLushort *values);
		fgl_api fgl_func_glPixelMapusv* fgl_glPixelMapusv;
#		define glPixelMapusv fgl_glPixelMapusv
		typedef void (FGL_APIENTRY fgl_func_glPixelStoref)(GLenum pname, GLfloat param);
		fgl_api fgl_func_glPixelStoref* fgl_glPixelStoref;
#		define glPixelStoref fgl_glPixelStoref
		typedef void (FGL_APIENTRY fgl_func_glPixelStorei)(GLenum pname, GLint param);
		fgl_api fgl_func_glPixelStorei* fgl_glPixelStorei;
#		define glPixelStorei fgl_glPixelStorei
		typedef void (FGL_APIENTRY fgl_func_glPixelTransferf)(GLenum pname, GLfloat param);
		fgl_api fgl_func_glPixelTransferf* fgl_glPixelTransferf;
#		define glPixelTransferf fgl_glPixelTransferf
		typedef void (FGL_APIENTRY fgl_func_glPixelTransferi)(GLenum pname, GLint param);
		fgl_api fgl_func_glPixelTransferi* fgl_glPixelTransferi;
#		define glPixelTransferi fgl_glPixelTransferi
		typedef void (FGL_APIENTRY fgl_func_glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
		fgl_api fgl_func_glPixelZoom* fgl_glPixelZoom;
#		define glPixelZoom fgl_glPixelZoom
		typedef void (FGL_APIENTRY fgl_func_glPointSize)(GLfloat size);
		fgl_api fgl_func_glPointSize* fgl_glPointSize;
#		define glPointSize fgl_glPointSize
		typedef void (FGL_APIENTRY fgl_func_glPolygonMode)(GLenum face, GLenum mode);
		fgl_api fgl_func_glPolygonMode* fgl_glPolygonMode;
#		define glPolygonMode fgl_glPolygonMode
		typedef void (FGL_APIENTRY fgl_func_glPolygonOffset)(GLfloat factor, GLfloat units);
		fgl_api fgl_func_glPolygonOffset* fgl_glPolygonOffset;
#		define glPolygonOffset fgl_glPolygonOffset
		typedef void (FGL_APIENTRY fgl_func_glPolygonStipple)(const GLubyte *mask);
		fgl_api fgl_func_glPolygonStipple* fgl_glPolygonStipple;
#		define glPolygonStipple fgl_glPolygonStipple
		typedef void (FGL_APIENTRY fgl_func_glPopAttrib)(void);
		fgl_api fgl_func_glPopAttrib* fgl_glPopAttrib;
#		define glPopAttrib fgl_glPopAttrib
		typedef void (FGL_APIENTRY fgl_func_glPopClientAttrib)(void);
		fgl_api fgl_func_glPopClientAttrib* fgl_glPopClientAttrib;
#		define glPopClientAttrib fgl_glPopClientAttrib
		typedef void (FGL_APIENTRY fgl_func_glPopMatrix)(void);
		fgl_api fgl_func_glPopMatrix* fgl_glPopMatrix;
#		define glPopMatrix fgl_glPopMatrix
		typedef void (FGL_APIENTRY fgl_func_glPopName)(void);
		fgl_api fgl_func_glPopName* fgl_glPopName;
#		define glPopName fgl_glPopName
		typedef void (FGL_APIENTRY fgl_func_glPrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
		fgl_api fgl_func_glPrioritizeTextures* fgl_glPrioritizeTextures;
#		define glPrioritizeTextures fgl_glPrioritizeTextures
		typedef void (FGL_APIENTRY fgl_func_glPushAttrib)(GLbitfield mask);
		fgl_api fgl_func_glPushAttrib* fgl_glPushAttrib;
#		define glPushAttrib fgl_glPushAttrib
		typedef void (FGL_APIENTRY fgl_func_glPushClientAttrib)(GLbitfield mask);
		fgl_api fgl_func_glPushClientAttrib* fgl_glPushClientAttrib;
#		define glPushClientAttrib fgl_glPushClientAttrib
		typedef void (FGL_APIENTRY fgl_func_glPushMatrix)(void);
		fgl_api fgl_func_glPushMatrix* fgl_glPushMatrix;
#		define glPushMatrix fgl_glPushMatrix
		typedef void (FGL_APIENTRY fgl_func_glPushName)(GLuint name);
		fgl_api fgl_func_glPushName* fgl_glPushName;
#		define glPushName fgl_glPushName
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2d)(GLdouble x, GLdouble y);
		fgl_api fgl_func_glRasterPos2d* fgl_glRasterPos2d;
#		define glRasterPos2d fgl_glRasterPos2d
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2dv)(const GLdouble *v);
		fgl_api fgl_func_glRasterPos2dv* fgl_glRasterPos2dv;
#		define glRasterPos2dv fgl_glRasterPos2dv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2f)(GLfloat x, GLfloat y);
		fgl_api fgl_func_glRasterPos2f* fgl_glRasterPos2f;
#		define glRasterPos2f fgl_glRasterPos2f
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2fv)(const GLfloat *v);
		fgl_api fgl_func_glRasterPos2fv* fgl_glRasterPos2fv;
#		define glRasterPos2fv fgl_glRasterPos2fv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2i)(GLint x, GLint y);
		fgl_api fgl_func_glRasterPos2i* fgl_glRasterPos2i;
#		define glRasterPos2i fgl_glRasterPos2i
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2iv)(const GLint *v);
		fgl_api fgl_func_glRasterPos2iv* fgl_glRasterPos2iv;
#		define glRasterPos2iv fgl_glRasterPos2iv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2s)(GLshort x, GLshort y);
		fgl_api fgl_func_glRasterPos2s* fgl_glRasterPos2s;
#		define glRasterPos2s fgl_glRasterPos2s
		typedef void (FGL_APIENTRY fgl_func_glRasterPos2sv)(const GLshort *v);
		fgl_api fgl_func_glRasterPos2sv* fgl_glRasterPos2sv;
#		define glRasterPos2sv fgl_glRasterPos2sv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glRasterPos3d* fgl_glRasterPos3d;
#		define glRasterPos3d fgl_glRasterPos3d
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3dv)(const GLdouble *v);
		fgl_api fgl_func_glRasterPos3dv* fgl_glRasterPos3dv;
#		define glRasterPos3dv fgl_glRasterPos3dv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
		fgl_api fgl_func_glRasterPos3f* fgl_glRasterPos3f;
#		define glRasterPos3f fgl_glRasterPos3f
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3fv)(const GLfloat *v);
		fgl_api fgl_func_glRasterPos3fv* fgl_glRasterPos3fv;
#		define glRasterPos3fv fgl_glRasterPos3fv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3i)(GLint x, GLint y, GLint z);
		fgl_api fgl_func_glRasterPos3i* fgl_glRasterPos3i;
#		define glRasterPos3i fgl_glRasterPos3i
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3iv)(const GLint *v);
		fgl_api fgl_func_glRasterPos3iv* fgl_glRasterPos3iv;
#		define glRasterPos3iv fgl_glRasterPos3iv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3s)(GLshort x, GLshort y, GLshort z);
		fgl_api fgl_func_glRasterPos3s* fgl_glRasterPos3s;
#		define glRasterPos3s fgl_glRasterPos3s
		typedef void (FGL_APIENTRY fgl_func_glRasterPos3sv)(const GLshort *v);
		fgl_api fgl_func_glRasterPos3sv* fgl_glRasterPos3sv;
#		define glRasterPos3sv fgl_glRasterPos3sv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		fgl_api fgl_func_glRasterPos4d* fgl_glRasterPos4d;
#		define glRasterPos4d fgl_glRasterPos4d
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4dv)(const GLdouble *v);
		fgl_api fgl_func_glRasterPos4dv* fgl_glRasterPos4dv;
#		define glRasterPos4dv fgl_glRasterPos4dv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		fgl_api fgl_func_glRasterPos4f* fgl_glRasterPos4f;
#		define glRasterPos4f fgl_glRasterPos4f
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4fv)(const GLfloat *v);
		fgl_api fgl_func_glRasterPos4fv* fgl_glRasterPos4fv;
#		define glRasterPos4fv fgl_glRasterPos4fv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
		fgl_api fgl_func_glRasterPos4i* fgl_glRasterPos4i;
#		define glRasterPos4i fgl_glRasterPos4i
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4iv)(const GLint *v);
		fgl_api fgl_func_glRasterPos4iv* fgl_glRasterPos4iv;
#		define glRasterPos4iv fgl_glRasterPos4iv
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
		fgl_api fgl_func_glRasterPos4s* fgl_glRasterPos4s;
#		define glRasterPos4s fgl_glRasterPos4s
		typedef void (FGL_APIENTRY fgl_func_glRasterPos4sv)(const GLshort *v);
		fgl_api fgl_func_glRasterPos4sv* fgl_glRasterPos4sv;
#		define glRasterPos4sv fgl_glRasterPos4sv
		typedef void (FGL_APIENTRY fgl_func_glReadBuffer)(GLenum mode);
		fgl_api fgl_func_glReadBuffer* fgl_glReadBuffer;
#		define glReadBuffer fgl_glReadBuffer
		typedef void (FGL_APIENTRY fgl_func_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
		fgl_api fgl_func_glReadPixels* fgl_glReadPixels;
#		define glReadPixels fgl_glReadPixels
		typedef void (FGL_APIENTRY fgl_func_glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
		fgl_api fgl_func_glRectd* fgl_glRectd;
#		define glRectd fgl_glRectd
		typedef void (FGL_APIENTRY fgl_func_glRectdv)(const GLdouble *v1, const GLdouble *v2);
		fgl_api fgl_func_glRectdv* fgl_glRectdv;
#		define glRectdv fgl_glRectdv
		typedef void (FGL_APIENTRY fgl_func_glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
		fgl_api fgl_func_glRectf* fgl_glRectf;
#		define glRectf fgl_glRectf
		typedef void (FGL_APIENTRY fgl_func_glRectfv)(const GLfloat *v1, const GLfloat *v2);
		fgl_api fgl_func_glRectfv* fgl_glRectfv;
#		define glRectfv fgl_glRectfv
		typedef void (FGL_APIENTRY fgl_func_glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
		fgl_api fgl_func_glRecti* fgl_glRecti;
#		define glRecti fgl_glRecti
		typedef void (FGL_APIENTRY fgl_func_glRectiv)(const GLint *v1, const GLint *v2);
		fgl_api fgl_func_glRectiv* fgl_glRectiv;
#		define glRectiv fgl_glRectiv
		typedef void (FGL_APIENTRY fgl_func_glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
		fgl_api fgl_func_glRects* fgl_glRects;
#		define glRects fgl_glRects
		typedef void (FGL_APIENTRY fgl_func_glRectsv)(const GLshort *v1, const GLshort *v2);
		fgl_api fgl_func_glRectsv* fgl_glRectsv;
#		define glRectsv fgl_glRectsv
		typedef GLint (FGL_APIENTRY fgl_func_glRenderMode)(GLenum mode);
		fgl_api fgl_func_glRenderMode* fgl_glRenderMode;
#		define glRenderMode fgl_glRenderMode
		typedef void (FGL_APIENTRY fgl_func_glRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glRotated* fgl_glRotated;
#		define glRotated fgl_glRotated
		typedef void (FGL_APIENTRY fgl_func_glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
		fgl_api fgl_func_glRotatef* fgl_glRotatef;
#		define glRotatef fgl_glRotatef
		typedef void (FGL_APIENTRY fgl_func_glScaled)(GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glScaled* fgl_glScaled;
#		define glScaled fgl_glScaled
		typedef void (FGL_APIENTRY fgl_func_glScalef)(GLfloat x, GLfloat y, GLfloat z);
		fgl_api fgl_func_glScalef* fgl_glScalef;
#		define glScalef fgl_glScalef
		typedef void (FGL_APIENTRY fgl_func_glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glScissor* fgl_glScissor;
#		define glScissor fgl_glScissor
		typedef void (FGL_APIENTRY fgl_func_glSelectBuffer)(GLsizei size, GLuint *buffer);
		fgl_api fgl_func_glSelectBuffer* fgl_glSelectBuffer;
#		define glSelectBuffer fgl_glSelectBuffer
		typedef void (FGL_APIENTRY fgl_func_glShadeModel)(GLenum mode);
		fgl_api fgl_func_glShadeModel* fgl_glShadeModel;
#		define glShadeModel fgl_glShadeModel
		typedef void (FGL_APIENTRY fgl_func_glStencilFunc)(GLenum func, GLint ref, GLuint mask);
		fgl_api fgl_func_glStencilFunc* fgl_glStencilFunc;
#		define glStencilFunc fgl_glStencilFunc
		typedef void (FGL_APIENTRY fgl_func_glStencilMask)(GLuint mask);
		fgl_api fgl_func_glStencilMask* fgl_glStencilMask;
#		define glStencilMask fgl_glStencilMask
		typedef void (FGL_APIENTRY fgl_func_glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
		fgl_api fgl_func_glStencilOp* fgl_glStencilOp;
#		define glStencilOp fgl_glStencilOp
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1d)(GLdouble s);
		fgl_api fgl_func_glTexCoord1d* fgl_glTexCoord1d;
#		define glTexCoord1d fgl_glTexCoord1d
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1dv)(const GLdouble *v);
		fgl_api fgl_func_glTexCoord1dv* fgl_glTexCoord1dv;
#		define glTexCoord1dv fgl_glTexCoord1dv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1f)(GLfloat s);
		fgl_api fgl_func_glTexCoord1f* fgl_glTexCoord1f;
#		define glTexCoord1f fgl_glTexCoord1f
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1fv)(const GLfloat *v);
		fgl_api fgl_func_glTexCoord1fv* fgl_glTexCoord1fv;
#		define glTexCoord1fv fgl_glTexCoord1fv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1i)(GLint s);
		fgl_api fgl_func_glTexCoord1i* fgl_glTexCoord1i;
#		define glTexCoord1i fgl_glTexCoord1i
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1iv)(const GLint *v);
		fgl_api fgl_func_glTexCoord1iv* fgl_glTexCoord1iv;
#		define glTexCoord1iv fgl_glTexCoord1iv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1s)(GLshort s);
		fgl_api fgl_func_glTexCoord1s* fgl_glTexCoord1s;
#		define glTexCoord1s fgl_glTexCoord1s
		typedef void (FGL_APIENTRY fgl_func_glTexCoord1sv)(const GLshort *v);
		fgl_api fgl_func_glTexCoord1sv* fgl_glTexCoord1sv;
#		define glTexCoord1sv fgl_glTexCoord1sv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2d)(GLdouble s, GLdouble t);
		fgl_api fgl_func_glTexCoord2d* fgl_glTexCoord2d;
#		define glTexCoord2d fgl_glTexCoord2d
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2dv)(const GLdouble *v);
		fgl_api fgl_func_glTexCoord2dv* fgl_glTexCoord2dv;
#		define glTexCoord2dv fgl_glTexCoord2dv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2f)(GLfloat s, GLfloat t);
		fgl_api fgl_func_glTexCoord2f* fgl_glTexCoord2f;
#		define glTexCoord2f fgl_glTexCoord2f
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2fv)(const GLfloat *v);
		fgl_api fgl_func_glTexCoord2fv* fgl_glTexCoord2fv;
#		define glTexCoord2fv fgl_glTexCoord2fv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2i)(GLint s, GLint t);
		fgl_api fgl_func_glTexCoord2i* fgl_glTexCoord2i;
#		define glTexCoord2i fgl_glTexCoord2i
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2iv)(const GLint *v);
		fgl_api fgl_func_glTexCoord2iv* fgl_glTexCoord2iv;
#		define glTexCoord2iv fgl_glTexCoord2iv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2s)(GLshort s, GLshort t);
		fgl_api fgl_func_glTexCoord2s* fgl_glTexCoord2s;
#		define glTexCoord2s fgl_glTexCoord2s
		typedef void (FGL_APIENTRY fgl_func_glTexCoord2sv)(const GLshort *v);
		fgl_api fgl_func_glTexCoord2sv* fgl_glTexCoord2sv;
#		define glTexCoord2sv fgl_glTexCoord2sv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
		fgl_api fgl_func_glTexCoord3d* fgl_glTexCoord3d;
#		define glTexCoord3d fgl_glTexCoord3d
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3dv)(const GLdouble *v);
		fgl_api fgl_func_glTexCoord3dv* fgl_glTexCoord3dv;
#		define glTexCoord3dv fgl_glTexCoord3dv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
		fgl_api fgl_func_glTexCoord3f* fgl_glTexCoord3f;
#		define glTexCoord3f fgl_glTexCoord3f
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3fv)(const GLfloat *v);
		fgl_api fgl_func_glTexCoord3fv* fgl_glTexCoord3fv;
#		define glTexCoord3fv fgl_glTexCoord3fv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3i)(GLint s, GLint t, GLint r);
		fgl_api fgl_func_glTexCoord3i* fgl_glTexCoord3i;
#		define glTexCoord3i fgl_glTexCoord3i
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3iv)(const GLint *v);
		fgl_api fgl_func_glTexCoord3iv* fgl_glTexCoord3iv;
#		define glTexCoord3iv fgl_glTexCoord3iv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3s)(GLshort s, GLshort t, GLshort r);
		fgl_api fgl_func_glTexCoord3s* fgl_glTexCoord3s;
#		define glTexCoord3s fgl_glTexCoord3s
		typedef void (FGL_APIENTRY fgl_func_glTexCoord3sv)(const GLshort *v);
		fgl_api fgl_func_glTexCoord3sv* fgl_glTexCoord3sv;
#		define glTexCoord3sv fgl_glTexCoord3sv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
		fgl_api fgl_func_glTexCoord4d* fgl_glTexCoord4d;
#		define glTexCoord4d fgl_glTexCoord4d
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4dv)(const GLdouble *v);
		fgl_api fgl_func_glTexCoord4dv* fgl_glTexCoord4dv;
#		define glTexCoord4dv fgl_glTexCoord4dv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
		fgl_api fgl_func_glTexCoord4f* fgl_glTexCoord4f;
#		define glTexCoord4f fgl_glTexCoord4f
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4fv)(const GLfloat *v);
		fgl_api fgl_func_glTexCoord4fv* fgl_glTexCoord4fv;
#		define glTexCoord4fv fgl_glTexCoord4fv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
		fgl_api fgl_func_glTexCoord4i* fgl_glTexCoord4i;
#		define glTexCoord4i fgl_glTexCoord4i
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4iv)(const GLint *v);
		fgl_api fgl_func_glTexCoord4iv* fgl_glTexCoord4iv;
#		define glTexCoord4iv fgl_glTexCoord4iv
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
		fgl_api fgl_func_glTexCoord4s* fgl_glTexCoord4s;
#		define glTexCoord4s fgl_glTexCoord4s
		typedef void (FGL_APIENTRY fgl_func_glTexCoord4sv)(const GLshort *v);
		fgl_api fgl_func_glTexCoord4sv* fgl_glTexCoord4sv;
#		define glTexCoord4sv fgl_glTexCoord4sv
		typedef void (FGL_APIENTRY fgl_func_glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		fgl_api fgl_func_glTexCoordPointer* fgl_glTexCoordPointer;
#		define glTexCoordPointer fgl_glTexCoordPointer
		typedef void (FGL_APIENTRY fgl_func_glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
		fgl_api fgl_func_glTexEnvf* fgl_glTexEnvf;
#		define glTexEnvf fgl_glTexEnvf
		typedef void (FGL_APIENTRY fgl_func_glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glTexEnvfv* fgl_glTexEnvfv;
#		define glTexEnvfv fgl_glTexEnvfv
		typedef void (FGL_APIENTRY fgl_func_glTexEnvi)(GLenum target, GLenum pname, GLint param);
		fgl_api fgl_func_glTexEnvi* fgl_glTexEnvi;
#		define glTexEnvi fgl_glTexEnvi
		typedef void (FGL_APIENTRY fgl_func_glTexEnviv)(GLenum target, GLenum pname, const GLint *params);
		fgl_api fgl_func_glTexEnviv* fgl_glTexEnviv;
#		define glTexEnviv fgl_glTexEnviv
		typedef void (FGL_APIENTRY fgl_func_glTexGend)(GLenum coord, GLenum pname, GLdouble param);
		fgl_api fgl_func_glTexGend* fgl_glTexGend;
#		define glTexGend fgl_glTexGend
		typedef void (FGL_APIENTRY fgl_func_glTexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
		fgl_api fgl_func_glTexGendv* fgl_glTexGendv;
#		define glTexGendv fgl_glTexGendv
		typedef void (FGL_APIENTRY fgl_func_glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
		fgl_api fgl_func_glTexGenf* fgl_glTexGenf;
#		define glTexGenf fgl_glTexGenf
		typedef void (FGL_APIENTRY fgl_func_glTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glTexGenfv* fgl_glTexGenfv;
#		define glTexGenfv fgl_glTexGenfv
		typedef void (FGL_APIENTRY fgl_func_glTexGeni)(GLenum coord, GLenum pname, GLint param);
		fgl_api fgl_func_glTexGeni* fgl_glTexGeni;
#		define glTexGeni fgl_glTexGeni
		typedef void (FGL_APIENTRY fgl_func_glTexGeniv)(GLenum coord, GLenum pname, const GLint *params);
		fgl_api fgl_func_glTexGeniv* fgl_glTexGeniv;
#		define glTexGeniv fgl_glTexGeniv
		typedef void (FGL_APIENTRY fgl_func_glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
		fgl_api fgl_func_glTexImage1D* fgl_glTexImage1D;
#		define glTexImage1D fgl_glTexImage1D
		typedef void (FGL_APIENTRY fgl_func_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
		fgl_api fgl_func_glTexImage2D* fgl_glTexImage2D;
#		define glTexImage2D fgl_glTexImage2D
		typedef void (FGL_APIENTRY fgl_func_glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
		fgl_api fgl_func_glTexParameterf* fgl_glTexParameterf;
#		define glTexParameterf fgl_glTexParameterf
		typedef void (FGL_APIENTRY fgl_func_glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glTexParameterfv* fgl_glTexParameterfv;
#		define glTexParameterfv fgl_glTexParameterfv
		typedef void (FGL_APIENTRY fgl_func_glTexParameteri)(GLenum target, GLenum pname, GLint param);
		fgl_api fgl_func_glTexParameteri* fgl_glTexParameteri;
#		define glTexParameteri fgl_glTexParameteri
		typedef void (FGL_APIENTRY fgl_func_glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
		fgl_api fgl_func_glTexParameteriv* fgl_glTexParameteriv;
#		define glTexParameteriv fgl_glTexParameteriv
		typedef void (FGL_APIENTRY fgl_func_glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
		fgl_api fgl_func_glTexSubImage1D* fgl_glTexSubImage1D;
#		define glTexSubImage1D fgl_glTexSubImage1D
		typedef void (FGL_APIENTRY fgl_func_glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
		fgl_api fgl_func_glTexSubImage2D* fgl_glTexSubImage2D;
#		define glTexSubImage2D fgl_glTexSubImage2D
		typedef void (FGL_APIENTRY fgl_func_glTranslated)(GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glTranslated* fgl_glTranslated;
#		define glTranslated fgl_glTranslated
		typedef void (FGL_APIENTRY fgl_func_glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
		fgl_api fgl_func_glTranslatef* fgl_glTranslatef;
#		define glTranslatef fgl_glTranslatef
		typedef void (FGL_APIENTRY fgl_func_glVertex2d)(GLdouble x, GLdouble y);
		fgl_api fgl_func_glVertex2d* fgl_glVertex2d;
#		define glVertex2d fgl_glVertex2d
		typedef void (FGL_APIENTRY fgl_func_glVertex2dv)(const GLdouble *v);
		fgl_api fgl_func_glVertex2dv* fgl_glVertex2dv;
#		define glVertex2dv fgl_glVertex2dv
		typedef void (FGL_APIENTRY fgl_func_glVertex2f)(GLfloat x, GLfloat y);
		fgl_api fgl_func_glVertex2f* fgl_glVertex2f;
#		define glVertex2f fgl_glVertex2f
		typedef void (FGL_APIENTRY fgl_func_glVertex2fv)(const GLfloat *v);
		fgl_api fgl_func_glVertex2fv* fgl_glVertex2fv;
#		define glVertex2fv fgl_glVertex2fv
		typedef void (FGL_APIENTRY fgl_func_glVertex2i)(GLint x, GLint y);
		fgl_api fgl_func_glVertex2i* fgl_glVertex2i;
#		define glVertex2i fgl_glVertex2i
		typedef void (FGL_APIENTRY fgl_func_glVertex2iv)(const GLint *v);
		fgl_api fgl_func_glVertex2iv* fgl_glVertex2iv;
#		define glVertex2iv fgl_glVertex2iv
		typedef void (FGL_APIENTRY fgl_func_glVertex2s)(GLshort x, GLshort y);
		fgl_api fgl_func_glVertex2s* fgl_glVertex2s;
#		define glVertex2s fgl_glVertex2s
		typedef void (FGL_APIENTRY fgl_func_glVertex2sv)(const GLshort *v);
		fgl_api fgl_func_glVertex2sv* fgl_glVertex2sv;
#		define glVertex2sv fgl_glVertex2sv
		typedef void (FGL_APIENTRY fgl_func_glVertex3d)(GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glVertex3d* fgl_glVertex3d;
#		define glVertex3d fgl_glVertex3d
		typedef void (FGL_APIENTRY fgl_func_glVertex3dv)(const GLdouble *v);
		fgl_api fgl_func_glVertex3dv* fgl_glVertex3dv;
#		define glVertex3dv fgl_glVertex3dv
		typedef void (FGL_APIENTRY fgl_func_glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
		fgl_api fgl_func_glVertex3f* fgl_glVertex3f;
#		define glVertex3f fgl_glVertex3f
		typedef void (FGL_APIENTRY fgl_func_glVertex3fv)(const GLfloat *v);
		fgl_api fgl_func_glVertex3fv* fgl_glVertex3fv;
#		define glVertex3fv fgl_glVertex3fv
		typedef void (FGL_APIENTRY fgl_func_glVertex3i)(GLint x, GLint y, GLint z);
		fgl_api fgl_func_glVertex3i* fgl_glVertex3i;
#		define glVertex3i fgl_glVertex3i
		typedef void (FGL_APIENTRY fgl_func_glVertex3iv)(const GLint *v);
		fgl_api fgl_func_glVertex3iv* fgl_glVertex3iv;
#		define glVertex3iv fgl_glVertex3iv
		typedef void (FGL_APIENTRY fgl_func_glVertex3s)(GLshort x, GLshort y, GLshort z);
		fgl_api fgl_func_glVertex3s* fgl_glVertex3s;
#		define glVertex3s fgl_glVertex3s
		typedef void (FGL_APIENTRY fgl_func_glVertex3sv)(const GLshort *v);
		fgl_api fgl_func_glVertex3sv* fgl_glVertex3sv;
#		define glVertex3sv fgl_glVertex3sv
		typedef void (FGL_APIENTRY fgl_func_glVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		fgl_api fgl_func_glVertex4d* fgl_glVertex4d;
#		define glVertex4d fgl_glVertex4d
		typedef void (FGL_APIENTRY fgl_func_glVertex4dv)(const GLdouble *v);
		fgl_api fgl_func_glVertex4dv* fgl_glVertex4dv;
#		define glVertex4dv fgl_glVertex4dv
		typedef void (FGL_APIENTRY fgl_func_glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		fgl_api fgl_func_glVertex4f* fgl_glVertex4f;
#		define glVertex4f fgl_glVertex4f
		typedef void (FGL_APIENTRY fgl_func_glVertex4fv)(const GLfloat *v);
		fgl_api fgl_func_glVertex4fv* fgl_glVertex4fv;
#		define glVertex4fv fgl_glVertex4fv
		typedef void (FGL_APIENTRY fgl_func_glVertex4i)(GLint x, GLint y, GLint z, GLint w);
		fgl_api fgl_func_glVertex4i* fgl_glVertex4i;
#		define glVertex4i fgl_glVertex4i
		typedef void (FGL_APIENTRY fgl_func_glVertex4iv)(const GLint *v);
		fgl_api fgl_func_glVertex4iv* fgl_glVertex4iv;
#		define glVertex4iv fgl_glVertex4iv
		typedef void (FGL_APIENTRY fgl_func_glVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
		fgl_api fgl_func_glVertex4s* fgl_glVertex4s;
#		define glVertex4s fgl_glVertex4s
		typedef void (FGL_APIENTRY fgl_func_glVertex4sv)(const GLshort *v);
		fgl_api fgl_func_glVertex4sv* fgl_glVertex4sv;
#		define glVertex4sv fgl_glVertex4sv
		typedef void (FGL_APIENTRY fgl_func_glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		fgl_api fgl_func_glVertexPointer* fgl_glVertexPointer;
#		define glVertexPointer fgl_glVertexPointer
		typedef void (FGL_APIENTRY fgl_func_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glViewport* fgl_glViewport;
#		define glViewport fgl_glViewport
#	endif // GL_VERSION_1_1

#	ifndef GL_VERSION_1_2
#		define GL_VERSION_1_2 1
		fgl_api bool isGL_VERSION_1_2;

#		define GL_UNSIGNED_BYTE_3_3_2 0x8032
#		define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#		define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#		define GL_UNSIGNED_INT_8_8_8_8 0x8035
#		define GL_UNSIGNED_INT_10_10_10_2 0x8036
#		define GL_TEXTURE_BINDING_3D 0x806A
#		define GL_PACK_SKIP_IMAGES 0x806B
#		define GL_PACK_IMAGE_HEIGHT 0x806C
#		define GL_UNPACK_SKIP_IMAGES 0x806D
#		define GL_UNPACK_IMAGE_HEIGHT 0x806E
#		define GL_TEXTURE_3D 0x806F
#		define GL_PROXY_TEXTURE_3D 0x8070
#		define GL_TEXTURE_DEPTH 0x8071
#		define GL_TEXTURE_WRAP_R 0x8072
#		define GL_MAX_3D_TEXTURE_SIZE 0x8073
#		define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#		define GL_UNSIGNED_SHORT_5_6_5 0x8363
#		define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#		define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#		define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#		define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#		define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#		define GL_BGR 0x80E0
#		define GL_BGRA 0x80E1
#		define GL_MAX_ELEMENTS_VERTICES 0x80E8
#		define GL_MAX_ELEMENTS_INDICES 0x80E9
#		define GL_CLAMP_TO_EDGE 0x812F
#		define GL_TEXTURE_MIN_LOD 0x813A
#		define GL_TEXTURE_MAX_LOD 0x813B
#		define GL_TEXTURE_BASE_LEVEL 0x813C
#		define GL_TEXTURE_MAX_LEVEL 0x813D
#		define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#		define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#		define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#		define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#		define GL_ALIASED_LINE_WIDTH_RANGE 0x846E
#		define GL_RESCALE_NORMAL 0x803A
#		define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#		define GL_SINGLE_COLOR 0x81F9
#		define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#		define GL_ALIASED_POINT_SIZE_RANGE 0x846D

		typedef void (FGL_APIENTRY fgl_func_glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
		fgl_api fgl_func_glDrawRangeElements* fgl_glDrawRangeElements;
#		define glDrawRangeElements fgl_glDrawRangeElements
		typedef void (FGL_APIENTRY fgl_func_glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
		fgl_api fgl_func_glTexImage3D* fgl_glTexImage3D;
#		define glTexImage3D fgl_glTexImage3D
		typedef void (FGL_APIENTRY fgl_func_glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
		fgl_api fgl_func_glTexSubImage3D* fgl_glTexSubImage3D;
#		define glTexSubImage3D fgl_glTexSubImage3D
		typedef void (FGL_APIENTRY fgl_func_glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glCopyTexSubImage3D* fgl_glCopyTexSubImage3D;
#		define glCopyTexSubImage3D fgl_glCopyTexSubImage3D
#	endif // GL_VERSION_1_2

#	ifndef GL_VERSION_1_3
#		define GL_VERSION_1_3 1
		fgl_api bool isGL_VERSION_1_3;

#		define GL_TEXTURE0 0x84C0
#		define GL_TEXTURE1 0x84C1
#		define GL_TEXTURE2 0x84C2
#		define GL_TEXTURE3 0x84C3
#		define GL_TEXTURE4 0x84C4
#		define GL_TEXTURE5 0x84C5
#		define GL_TEXTURE6 0x84C6
#		define GL_TEXTURE7 0x84C7
#		define GL_TEXTURE8 0x84C8
#		define GL_TEXTURE9 0x84C9
#		define GL_TEXTURE10 0x84CA
#		define GL_TEXTURE11 0x84CB
#		define GL_TEXTURE12 0x84CC
#		define GL_TEXTURE13 0x84CD
#		define GL_TEXTURE14 0x84CE
#		define GL_TEXTURE15 0x84CF
#		define GL_TEXTURE16 0x84D0
#		define GL_TEXTURE17 0x84D1
#		define GL_TEXTURE18 0x84D2
#		define GL_TEXTURE19 0x84D3
#		define GL_TEXTURE20 0x84D4
#		define GL_TEXTURE21 0x84D5
#		define GL_TEXTURE22 0x84D6
#		define GL_TEXTURE23 0x84D7
#		define GL_TEXTURE24 0x84D8
#		define GL_TEXTURE25 0x84D9
#		define GL_TEXTURE26 0x84DA
#		define GL_TEXTURE27 0x84DB
#		define GL_TEXTURE28 0x84DC
#		define GL_TEXTURE29 0x84DD
#		define GL_TEXTURE30 0x84DE
#		define GL_TEXTURE31 0x84DF
#		define GL_ACTIVE_TEXTURE 0x84E0
#		define GL_MULTISAMPLE 0x809D
#		define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#		define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#		define GL_SAMPLE_COVERAGE 0x80A0
#		define GL_SAMPLE_BUFFERS 0x80A8
#		define GL_SAMPLES 0x80A9
#		define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#		define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#		define GL_TEXTURE_CUBE_MAP 0x8513
#		define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#		define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#		define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#		define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#		define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#		define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#		define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#		define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#		define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#		define GL_COMPRESSED_RGB 0x84ED
#		define GL_COMPRESSED_RGBA 0x84EE
#		define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#		define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#		define GL_TEXTURE_COMPRESSED 0x86A1
#		define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#		define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#		define GL_CLAMP_TO_BORDER 0x812D
#		define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#		define GL_MAX_TEXTURE_UNITS 0x84E2
#		define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#		define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#		define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5
#		define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#		define GL_MULTISAMPLE_BIT 0x20000000
#		define GL_NORMAL_MAP 0x8511
#		define GL_REFLECTION_MAP 0x8512
#		define GL_COMPRESSED_ALPHA 0x84E9
#		define GL_COMPRESSED_LUMINANCE 0x84EA
#		define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#		define GL_COMPRESSED_INTENSITY 0x84EC
#		define GL_COMBINE 0x8570
#		define GL_COMBINE_RGB 0x8571
#		define GL_COMBINE_ALPHA 0x8572
#		define GL_SOURCE0_RGB 0x8580
#		define GL_SOURCE1_RGB 0x8581
#		define GL_SOURCE2_RGB 0x8582
#		define GL_SOURCE0_ALPHA 0x8588
#		define GL_SOURCE1_ALPHA 0x8589
#		define GL_SOURCE2_ALPHA 0x858A
#		define GL_OPERAND0_RGB 0x8590
#		define GL_OPERAND1_RGB 0x8591
#		define GL_OPERAND2_RGB 0x8592
#		define GL_OPERAND0_ALPHA 0x8598
#		define GL_OPERAND1_ALPHA 0x8599
#		define GL_OPERAND2_ALPHA 0x859A
#		define GL_RGB_SCALE 0x8573
#		define GL_ADD_SIGNED 0x8574
#		define GL_INTERPOLATE 0x8575
#		define GL_SUBTRACT 0x84E7
#		define GL_CONSTANT 0x8576
#		define GL_PRIMARY_COLOR 0x8577
#		define GL_PREVIOUS 0x8578
#		define GL_DOT3_RGB 0x86AE
#		define GL_DOT3_RGBA 0x86AF

		typedef void (FGL_APIENTRY fgl_func_glActiveTexture)(GLenum texture);
		fgl_api fgl_func_glActiveTexture* fgl_glActiveTexture;
#		define glActiveTexture fgl_glActiveTexture
		typedef void (FGL_APIENTRY fgl_func_glSampleCoverage)(GLfloat value, GLboolean invert);
		fgl_api fgl_func_glSampleCoverage* fgl_glSampleCoverage;
#		define glSampleCoverage fgl_glSampleCoverage
		typedef void (FGL_APIENTRY fgl_func_glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTexImage3D* fgl_glCompressedTexImage3D;
#		define glCompressedTexImage3D fgl_glCompressedTexImage3D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTexImage2D* fgl_glCompressedTexImage2D;
#		define glCompressedTexImage2D fgl_glCompressedTexImage2D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTexImage1D* fgl_glCompressedTexImage1D;
#		define glCompressedTexImage1D fgl_glCompressedTexImage1D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTexSubImage3D* fgl_glCompressedTexSubImage3D;
#		define glCompressedTexSubImage3D fgl_glCompressedTexSubImage3D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTexSubImage2D* fgl_glCompressedTexSubImage2D;
#		define glCompressedTexSubImage2D fgl_glCompressedTexSubImage2D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTexSubImage1D* fgl_glCompressedTexSubImage1D;
#		define glCompressedTexSubImage1D fgl_glCompressedTexSubImage1D
		typedef void (FGL_APIENTRY fgl_func_glGetCompressedTexImage)(GLenum target, GLint level, void *img);
		fgl_api fgl_func_glGetCompressedTexImage* fgl_glGetCompressedTexImage;
#		define glGetCompressedTexImage fgl_glGetCompressedTexImage
		typedef void (FGL_APIENTRY fgl_func_glClientActiveTexture)(GLenum texture);
		fgl_api fgl_func_glClientActiveTexture* fgl_glClientActiveTexture;
#		define glClientActiveTexture fgl_glClientActiveTexture
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1d)(GLenum target, GLdouble s);
		fgl_api fgl_func_glMultiTexCoord1d* fgl_glMultiTexCoord1d;
#		define glMultiTexCoord1d fgl_glMultiTexCoord1d
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1dv)(GLenum target, const GLdouble *v);
		fgl_api fgl_func_glMultiTexCoord1dv* fgl_glMultiTexCoord1dv;
#		define glMultiTexCoord1dv fgl_glMultiTexCoord1dv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1f)(GLenum target, GLfloat s);
		fgl_api fgl_func_glMultiTexCoord1f* fgl_glMultiTexCoord1f;
#		define glMultiTexCoord1f fgl_glMultiTexCoord1f
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1fv)(GLenum target, const GLfloat *v);
		fgl_api fgl_func_glMultiTexCoord1fv* fgl_glMultiTexCoord1fv;
#		define glMultiTexCoord1fv fgl_glMultiTexCoord1fv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1i)(GLenum target, GLint s);
		fgl_api fgl_func_glMultiTexCoord1i* fgl_glMultiTexCoord1i;
#		define glMultiTexCoord1i fgl_glMultiTexCoord1i
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1iv)(GLenum target, const GLint *v);
		fgl_api fgl_func_glMultiTexCoord1iv* fgl_glMultiTexCoord1iv;
#		define glMultiTexCoord1iv fgl_glMultiTexCoord1iv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1s)(GLenum target, GLshort s);
		fgl_api fgl_func_glMultiTexCoord1s* fgl_glMultiTexCoord1s;
#		define glMultiTexCoord1s fgl_glMultiTexCoord1s
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord1sv)(GLenum target, const GLshort *v);
		fgl_api fgl_func_glMultiTexCoord1sv* fgl_glMultiTexCoord1sv;
#		define glMultiTexCoord1sv fgl_glMultiTexCoord1sv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
		fgl_api fgl_func_glMultiTexCoord2d* fgl_glMultiTexCoord2d;
#		define glMultiTexCoord2d fgl_glMultiTexCoord2d
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2dv)(GLenum target, const GLdouble *v);
		fgl_api fgl_func_glMultiTexCoord2dv* fgl_glMultiTexCoord2dv;
#		define glMultiTexCoord2dv fgl_glMultiTexCoord2dv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
		fgl_api fgl_func_glMultiTexCoord2f* fgl_glMultiTexCoord2f;
#		define glMultiTexCoord2f fgl_glMultiTexCoord2f
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2fv)(GLenum target, const GLfloat *v);
		fgl_api fgl_func_glMultiTexCoord2fv* fgl_glMultiTexCoord2fv;
#		define glMultiTexCoord2fv fgl_glMultiTexCoord2fv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2i)(GLenum target, GLint s, GLint t);
		fgl_api fgl_func_glMultiTexCoord2i* fgl_glMultiTexCoord2i;
#		define glMultiTexCoord2i fgl_glMultiTexCoord2i
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2iv)(GLenum target, const GLint *v);
		fgl_api fgl_func_glMultiTexCoord2iv* fgl_glMultiTexCoord2iv;
#		define glMultiTexCoord2iv fgl_glMultiTexCoord2iv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
		fgl_api fgl_func_glMultiTexCoord2s* fgl_glMultiTexCoord2s;
#		define glMultiTexCoord2s fgl_glMultiTexCoord2s
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord2sv)(GLenum target, const GLshort *v);
		fgl_api fgl_func_glMultiTexCoord2sv* fgl_glMultiTexCoord2sv;
#		define glMultiTexCoord2sv fgl_glMultiTexCoord2sv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
		fgl_api fgl_func_glMultiTexCoord3d* fgl_glMultiTexCoord3d;
#		define glMultiTexCoord3d fgl_glMultiTexCoord3d
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3dv)(GLenum target, const GLdouble *v);
		fgl_api fgl_func_glMultiTexCoord3dv* fgl_glMultiTexCoord3dv;
#		define glMultiTexCoord3dv fgl_glMultiTexCoord3dv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
		fgl_api fgl_func_glMultiTexCoord3f* fgl_glMultiTexCoord3f;
#		define glMultiTexCoord3f fgl_glMultiTexCoord3f
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3fv)(GLenum target, const GLfloat *v);
		fgl_api fgl_func_glMultiTexCoord3fv* fgl_glMultiTexCoord3fv;
#		define glMultiTexCoord3fv fgl_glMultiTexCoord3fv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
		fgl_api fgl_func_glMultiTexCoord3i* fgl_glMultiTexCoord3i;
#		define glMultiTexCoord3i fgl_glMultiTexCoord3i
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3iv)(GLenum target, const GLint *v);
		fgl_api fgl_func_glMultiTexCoord3iv* fgl_glMultiTexCoord3iv;
#		define glMultiTexCoord3iv fgl_glMultiTexCoord3iv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
		fgl_api fgl_func_glMultiTexCoord3s* fgl_glMultiTexCoord3s;
#		define glMultiTexCoord3s fgl_glMultiTexCoord3s
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord3sv)(GLenum target, const GLshort *v);
		fgl_api fgl_func_glMultiTexCoord3sv* fgl_glMultiTexCoord3sv;
#		define glMultiTexCoord3sv fgl_glMultiTexCoord3sv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
		fgl_api fgl_func_glMultiTexCoord4d* fgl_glMultiTexCoord4d;
#		define glMultiTexCoord4d fgl_glMultiTexCoord4d
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4dv)(GLenum target, const GLdouble *v);
		fgl_api fgl_func_glMultiTexCoord4dv* fgl_glMultiTexCoord4dv;
#		define glMultiTexCoord4dv fgl_glMultiTexCoord4dv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
		fgl_api fgl_func_glMultiTexCoord4f* fgl_glMultiTexCoord4f;
#		define glMultiTexCoord4f fgl_glMultiTexCoord4f
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4fv)(GLenum target, const GLfloat *v);
		fgl_api fgl_func_glMultiTexCoord4fv* fgl_glMultiTexCoord4fv;
#		define glMultiTexCoord4fv fgl_glMultiTexCoord4fv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
		fgl_api fgl_func_glMultiTexCoord4i* fgl_glMultiTexCoord4i;
#		define glMultiTexCoord4i fgl_glMultiTexCoord4i
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4iv)(GLenum target, const GLint *v);
		fgl_api fgl_func_glMultiTexCoord4iv* fgl_glMultiTexCoord4iv;
#		define glMultiTexCoord4iv fgl_glMultiTexCoord4iv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
		fgl_api fgl_func_glMultiTexCoord4s* fgl_glMultiTexCoord4s;
#		define glMultiTexCoord4s fgl_glMultiTexCoord4s
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoord4sv)(GLenum target, const GLshort *v);
		fgl_api fgl_func_glMultiTexCoord4sv* fgl_glMultiTexCoord4sv;
#		define glMultiTexCoord4sv fgl_glMultiTexCoord4sv
		typedef void (FGL_APIENTRY fgl_func_glLoadTransposeMatrixf)(const GLfloat *m);
		fgl_api fgl_func_glLoadTransposeMatrixf* fgl_glLoadTransposeMatrixf;
#		define glLoadTransposeMatrixf fgl_glLoadTransposeMatrixf
		typedef void (FGL_APIENTRY fgl_func_glLoadTransposeMatrixd)(const GLdouble *m);
		fgl_api fgl_func_glLoadTransposeMatrixd* fgl_glLoadTransposeMatrixd;
#		define glLoadTransposeMatrixd fgl_glLoadTransposeMatrixd
		typedef void (FGL_APIENTRY fgl_func_glMultTransposeMatrixf)(const GLfloat *m);
		fgl_api fgl_func_glMultTransposeMatrixf* fgl_glMultTransposeMatrixf;
#		define glMultTransposeMatrixf fgl_glMultTransposeMatrixf
		typedef void (FGL_APIENTRY fgl_func_glMultTransposeMatrixd)(const GLdouble *m);
		fgl_api fgl_func_glMultTransposeMatrixd* fgl_glMultTransposeMatrixd;
#		define glMultTransposeMatrixd fgl_glMultTransposeMatrixd
#	endif // GL_VERSION_1_3

#	ifndef GL_VERSION_1_4
#		define GL_VERSION_1_4 1
		fgl_api bool isGL_VERSION_1_4;

#		define GL_BLEND_DST_RGB 0x80C8
#		define GL_BLEND_SRC_RGB 0x80C9
#		define GL_BLEND_DST_ALPHA 0x80CA
#		define GL_BLEND_SRC_ALPHA 0x80CB
#		define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#		define GL_DEPTH_COMPONENT16 0x81A5
#		define GL_DEPTH_COMPONENT24 0x81A6
#		define GL_DEPTH_COMPONENT32 0x81A7
#		define GL_MIRRORED_REPEAT 0x8370
#		define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#		define GL_TEXTURE_LOD_BIAS 0x8501
#		define GL_INCR_WRAP 0x8507
#		define GL_DECR_WRAP 0x8508
#		define GL_TEXTURE_DEPTH_SIZE 0x884A
#		define GL_TEXTURE_COMPARE_MODE 0x884C
#		define GL_TEXTURE_COMPARE_FUNC 0x884D
#		define GL_POINT_SIZE_MIN 0x8126
#		define GL_POINT_SIZE_MAX 0x8127
#		define GL_POINT_DISTANCE_ATTENUATION 0x8129
#		define GL_GENERATE_MIPMAP 0x8191
#		define GL_GENERATE_MIPMAP_HINT 0x8192
#		define GL_FOG_COORDINATE_SOURCE 0x8450
#		define GL_FOG_COORDINATE 0x8451
#		define GL_FRAGMENT_DEPTH 0x8452
#		define GL_CURRENT_FOG_COORDINATE 0x8453
#		define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#		define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#		define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#		define GL_FOG_COORDINATE_ARRAY 0x8457
#		define GL_COLOR_SUM 0x8458
#		define GL_CURRENT_SECONDARY_COLOR 0x8459
#		define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#		define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#		define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#		define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#		define GL_SECONDARY_COLOR_ARRAY 0x845E
#		define GL_TEXTURE_FILTER_CONTROL 0x8500
#		define GL_DEPTH_TEXTURE_MODE 0x884B
#		define GL_COMPARE_R_TO_TEXTURE 0x884E
#		define GL_BLEND_COLOR 0x8005
#		define GL_BLEND_EQUATION 0x8009
#		define GL_CONSTANT_COLOR 0x8001
#		define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#		define GL_CONSTANT_ALPHA 0x8003
#		define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#		define GL_FUNC_ADD 0x8006
#		define GL_FUNC_REVERSE_SUBTRACT 0x800B
#		define GL_FUNC_SUBTRACT 0x800A
#		define GL_MIN 0x8007
#		define GL_MAX 0x8008

		typedef void (FGL_APIENTRY fgl_func_glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
		fgl_api fgl_func_glBlendFuncSeparate* fgl_glBlendFuncSeparate;
#		define glBlendFuncSeparate fgl_glBlendFuncSeparate
		typedef void (FGL_APIENTRY fgl_func_glMultiDrawArrays)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
		fgl_api fgl_func_glMultiDrawArrays* fgl_glMultiDrawArrays;
#		define glMultiDrawArrays fgl_glMultiDrawArrays
		typedef void (FGL_APIENTRY fgl_func_glMultiDrawElements)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
		fgl_api fgl_func_glMultiDrawElements* fgl_glMultiDrawElements;
#		define glMultiDrawElements fgl_glMultiDrawElements
		typedef void (FGL_APIENTRY fgl_func_glPointParameterf)(GLenum pname, GLfloat param);
		fgl_api fgl_func_glPointParameterf* fgl_glPointParameterf;
#		define glPointParameterf fgl_glPointParameterf
		typedef void (FGL_APIENTRY fgl_func_glPointParameterfv)(GLenum pname, const GLfloat *params);
		fgl_api fgl_func_glPointParameterfv* fgl_glPointParameterfv;
#		define glPointParameterfv fgl_glPointParameterfv
		typedef void (FGL_APIENTRY fgl_func_glPointParameteri)(GLenum pname, GLint param);
		fgl_api fgl_func_glPointParameteri* fgl_glPointParameteri;
#		define glPointParameteri fgl_glPointParameteri
		typedef void (FGL_APIENTRY fgl_func_glPointParameteriv)(GLenum pname, const GLint *params);
		fgl_api fgl_func_glPointParameteriv* fgl_glPointParameteriv;
#		define glPointParameteriv fgl_glPointParameteriv
		typedef void (FGL_APIENTRY fgl_func_glFogCoordf)(GLfloat coord);
		fgl_api fgl_func_glFogCoordf* fgl_glFogCoordf;
#		define glFogCoordf fgl_glFogCoordf
		typedef void (FGL_APIENTRY fgl_func_glFogCoordfv)(const GLfloat *coord);
		fgl_api fgl_func_glFogCoordfv* fgl_glFogCoordfv;
#		define glFogCoordfv fgl_glFogCoordfv
		typedef void (FGL_APIENTRY fgl_func_glFogCoordd)(GLdouble coord);
		fgl_api fgl_func_glFogCoordd* fgl_glFogCoordd;
#		define glFogCoordd fgl_glFogCoordd
		typedef void (FGL_APIENTRY fgl_func_glFogCoorddv)(const GLdouble *coord);
		fgl_api fgl_func_glFogCoorddv* fgl_glFogCoorddv;
#		define glFogCoorddv fgl_glFogCoorddv
		typedef void (FGL_APIENTRY fgl_func_glFogCoordPointer)(GLenum type, GLsizei stride, const void *pointer);
		fgl_api fgl_func_glFogCoordPointer* fgl_glFogCoordPointer;
#		define glFogCoordPointer fgl_glFogCoordPointer
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue);
		fgl_api fgl_func_glSecondaryColor3b* fgl_glSecondaryColor3b;
#		define glSecondaryColor3b fgl_glSecondaryColor3b
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3bv)(const GLbyte *v);
		fgl_api fgl_func_glSecondaryColor3bv* fgl_glSecondaryColor3bv;
#		define glSecondaryColor3bv fgl_glSecondaryColor3bv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue);
		fgl_api fgl_func_glSecondaryColor3d* fgl_glSecondaryColor3d;
#		define glSecondaryColor3d fgl_glSecondaryColor3d
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3dv)(const GLdouble *v);
		fgl_api fgl_func_glSecondaryColor3dv* fgl_glSecondaryColor3dv;
#		define glSecondaryColor3dv fgl_glSecondaryColor3dv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue);
		fgl_api fgl_func_glSecondaryColor3f* fgl_glSecondaryColor3f;
#		define glSecondaryColor3f fgl_glSecondaryColor3f
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3fv)(const GLfloat *v);
		fgl_api fgl_func_glSecondaryColor3fv* fgl_glSecondaryColor3fv;
#		define glSecondaryColor3fv fgl_glSecondaryColor3fv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3i)(GLint red, GLint green, GLint blue);
		fgl_api fgl_func_glSecondaryColor3i* fgl_glSecondaryColor3i;
#		define glSecondaryColor3i fgl_glSecondaryColor3i
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3iv)(const GLint *v);
		fgl_api fgl_func_glSecondaryColor3iv* fgl_glSecondaryColor3iv;
#		define glSecondaryColor3iv fgl_glSecondaryColor3iv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3s)(GLshort red, GLshort green, GLshort blue);
		fgl_api fgl_func_glSecondaryColor3s* fgl_glSecondaryColor3s;
#		define glSecondaryColor3s fgl_glSecondaryColor3s
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3sv)(const GLshort *v);
		fgl_api fgl_func_glSecondaryColor3sv* fgl_glSecondaryColor3sv;
#		define glSecondaryColor3sv fgl_glSecondaryColor3sv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
		fgl_api fgl_func_glSecondaryColor3ub* fgl_glSecondaryColor3ub;
#		define glSecondaryColor3ub fgl_glSecondaryColor3ub
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3ubv)(const GLubyte *v);
		fgl_api fgl_func_glSecondaryColor3ubv* fgl_glSecondaryColor3ubv;
#		define glSecondaryColor3ubv fgl_glSecondaryColor3ubv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3ui)(GLuint red, GLuint green, GLuint blue);
		fgl_api fgl_func_glSecondaryColor3ui* fgl_glSecondaryColor3ui;
#		define glSecondaryColor3ui fgl_glSecondaryColor3ui
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3uiv)(const GLuint *v);
		fgl_api fgl_func_glSecondaryColor3uiv* fgl_glSecondaryColor3uiv;
#		define glSecondaryColor3uiv fgl_glSecondaryColor3uiv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3us)(GLushort red, GLushort green, GLushort blue);
		fgl_api fgl_func_glSecondaryColor3us* fgl_glSecondaryColor3us;
#		define glSecondaryColor3us fgl_glSecondaryColor3us
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColor3usv)(const GLushort *v);
		fgl_api fgl_func_glSecondaryColor3usv* fgl_glSecondaryColor3usv;
#		define glSecondaryColor3usv fgl_glSecondaryColor3usv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const void *pointer);
		fgl_api fgl_func_glSecondaryColorPointer* fgl_glSecondaryColorPointer;
#		define glSecondaryColorPointer fgl_glSecondaryColorPointer
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2d)(GLdouble x, GLdouble y);
		fgl_api fgl_func_glWindowPos2d* fgl_glWindowPos2d;
#		define glWindowPos2d fgl_glWindowPos2d
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2dv)(const GLdouble *v);
		fgl_api fgl_func_glWindowPos2dv* fgl_glWindowPos2dv;
#		define glWindowPos2dv fgl_glWindowPos2dv
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2f)(GLfloat x, GLfloat y);
		fgl_api fgl_func_glWindowPos2f* fgl_glWindowPos2f;
#		define glWindowPos2f fgl_glWindowPos2f
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2fv)(const GLfloat *v);
		fgl_api fgl_func_glWindowPos2fv* fgl_glWindowPos2fv;
#		define glWindowPos2fv fgl_glWindowPos2fv
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2i)(GLint x, GLint y);
		fgl_api fgl_func_glWindowPos2i* fgl_glWindowPos2i;
#		define glWindowPos2i fgl_glWindowPos2i
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2iv)(const GLint *v);
		fgl_api fgl_func_glWindowPos2iv* fgl_glWindowPos2iv;
#		define glWindowPos2iv fgl_glWindowPos2iv
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2s)(GLshort x, GLshort y);
		fgl_api fgl_func_glWindowPos2s* fgl_glWindowPos2s;
#		define glWindowPos2s fgl_glWindowPos2s
		typedef void (FGL_APIENTRY fgl_func_glWindowPos2sv)(const GLshort *v);
		fgl_api fgl_func_glWindowPos2sv* fgl_glWindowPos2sv;
#		define glWindowPos2sv fgl_glWindowPos2sv
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3d)(GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glWindowPos3d* fgl_glWindowPos3d;
#		define glWindowPos3d fgl_glWindowPos3d
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3dv)(const GLdouble *v);
		fgl_api fgl_func_glWindowPos3dv* fgl_glWindowPos3dv;
#		define glWindowPos3dv fgl_glWindowPos3dv
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3f)(GLfloat x, GLfloat y, GLfloat z);
		fgl_api fgl_func_glWindowPos3f* fgl_glWindowPos3f;
#		define glWindowPos3f fgl_glWindowPos3f
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3fv)(const GLfloat *v);
		fgl_api fgl_func_glWindowPos3fv* fgl_glWindowPos3fv;
#		define glWindowPos3fv fgl_glWindowPos3fv
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3i)(GLint x, GLint y, GLint z);
		fgl_api fgl_func_glWindowPos3i* fgl_glWindowPos3i;
#		define glWindowPos3i fgl_glWindowPos3i
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3iv)(const GLint *v);
		fgl_api fgl_func_glWindowPos3iv* fgl_glWindowPos3iv;
#		define glWindowPos3iv fgl_glWindowPos3iv
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3s)(GLshort x, GLshort y, GLshort z);
		fgl_api fgl_func_glWindowPos3s* fgl_glWindowPos3s;
#		define glWindowPos3s fgl_glWindowPos3s
		typedef void (FGL_APIENTRY fgl_func_glWindowPos3sv)(const GLshort *v);
		fgl_api fgl_func_glWindowPos3sv* fgl_glWindowPos3sv;
#		define glWindowPos3sv fgl_glWindowPos3sv
		typedef void (FGL_APIENTRY fgl_func_glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		fgl_api fgl_func_glBlendColor* fgl_glBlendColor;
#		define glBlendColor fgl_glBlendColor
		typedef void (FGL_APIENTRY fgl_func_glBlendEquation)(GLenum mode);
		fgl_api fgl_func_glBlendEquation* fgl_glBlendEquation;
#		define glBlendEquation fgl_glBlendEquation
#	endif // GL_VERSION_1_4

#	ifndef GL_VERSION_1_5
#		define GL_VERSION_1_5 1
		fgl_api bool isGL_VERSION_1_5;

		typedef ptrdiff_t GLsizeiptr;
		typedef ptrdiff_t GLintptr;

#		define GL_BUFFER_SIZE 0x8764
#		define GL_BUFFER_USAGE 0x8765
#		define GL_QUERY_COUNTER_BITS 0x8864
#		define GL_CURRENT_QUERY 0x8865
#		define GL_QUERY_RESULT 0x8866
#		define GL_QUERY_RESULT_AVAILABLE 0x8867
#		define GL_ARRAY_BUFFER 0x8892
#		define GL_ELEMENT_ARRAY_BUFFER 0x8893
#		define GL_ARRAY_BUFFER_BINDING 0x8894
#		define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#		define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#		define GL_READ_ONLY 0x88B8
#		define GL_WRITE_ONLY 0x88B9
#		define GL_READ_WRITE 0x88BA
#		define GL_BUFFER_ACCESS 0x88BB
#		define GL_BUFFER_MAPPED 0x88BC
#		define GL_BUFFER_MAP_POINTER 0x88BD
#		define GL_STREAM_DRAW 0x88E0
#		define GL_STREAM_READ 0x88E1
#		define GL_STREAM_COPY 0x88E2
#		define GL_STATIC_DRAW 0x88E4
#		define GL_STATIC_READ 0x88E5
#		define GL_STATIC_COPY 0x88E6
#		define GL_DYNAMIC_DRAW 0x88E8
#		define GL_DYNAMIC_READ 0x88E9
#		define GL_DYNAMIC_COPY 0x88EA
#		define GL_SAMPLES_PASSED 0x8914
#		define GL_SRC1_ALPHA 0x8589
#		define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#		define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#		define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#		define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#		define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#		define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#		define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#		define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#		define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#		define GL_FOG_COORD_SRC 0x8450
#		define GL_FOG_COORD 0x8451
#		define GL_CURRENT_FOG_COORD 0x8453
#		define GL_FOG_COORD_ARRAY_TYPE 0x8454
#		define GL_FOG_COORD_ARRAY_STRIDE 0x8455
#		define GL_FOG_COORD_ARRAY_POINTER 0x8456
#		define GL_FOG_COORD_ARRAY 0x8457
#		define GL_FOG_COORD_ARRAY_BUFFER_BINDING 0x889D
#		define GL_SRC0_RGB 0x8580
#		define GL_SRC1_RGB 0x8581
#		define GL_SRC2_RGB 0x8582
#		define GL_SRC0_ALPHA 0x8588
#		define GL_SRC2_ALPHA 0x858A

		typedef void (FGL_APIENTRY fgl_func_glGenQueries)(GLsizei n, GLuint *ids);
		fgl_api fgl_func_glGenQueries* fgl_glGenQueries;
#		define glGenQueries fgl_glGenQueries
		typedef void (FGL_APIENTRY fgl_func_glDeleteQueries)(GLsizei n, const GLuint *ids);
		fgl_api fgl_func_glDeleteQueries* fgl_glDeleteQueries;
#		define glDeleteQueries fgl_glDeleteQueries
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsQuery)(GLuint id);
		fgl_api fgl_func_glIsQuery* fgl_glIsQuery;
#		define glIsQuery fgl_glIsQuery
		typedef void (FGL_APIENTRY fgl_func_glBeginQuery)(GLenum target, GLuint id);
		fgl_api fgl_func_glBeginQuery* fgl_glBeginQuery;
#		define glBeginQuery fgl_glBeginQuery
		typedef void (FGL_APIENTRY fgl_func_glEndQuery)(GLenum target);
		fgl_api fgl_func_glEndQuery* fgl_glEndQuery;
#		define glEndQuery fgl_glEndQuery
		typedef void (FGL_APIENTRY fgl_func_glGetQueryiv)(GLenum target, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetQueryiv* fgl_glGetQueryiv;
#		define glGetQueryiv fgl_glGetQueryiv
		typedef void (FGL_APIENTRY fgl_func_glGetQueryObjectiv)(GLuint id, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetQueryObjectiv* fgl_glGetQueryObjectiv;
#		define glGetQueryObjectiv fgl_glGetQueryObjectiv
		typedef void (FGL_APIENTRY fgl_func_glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params);
		fgl_api fgl_func_glGetQueryObjectuiv* fgl_glGetQueryObjectuiv;
#		define glGetQueryObjectuiv fgl_glGetQueryObjectuiv
		typedef void (FGL_APIENTRY fgl_func_glBindBuffer)(GLenum target, GLuint buffer);
		fgl_api fgl_func_glBindBuffer* fgl_glBindBuffer;
#		define glBindBuffer fgl_glBindBuffer
		typedef void (FGL_APIENTRY fgl_func_glDeleteBuffers)(GLsizei n, const GLuint *buffers);
		fgl_api fgl_func_glDeleteBuffers* fgl_glDeleteBuffers;
#		define glDeleteBuffers fgl_glDeleteBuffers
		typedef void (FGL_APIENTRY fgl_func_glGenBuffers)(GLsizei n, GLuint *buffers);
		fgl_api fgl_func_glGenBuffers* fgl_glGenBuffers;
#		define glGenBuffers fgl_glGenBuffers
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsBuffer)(GLuint buffer);
		fgl_api fgl_func_glIsBuffer* fgl_glIsBuffer;
#		define glIsBuffer fgl_glIsBuffer
		typedef void (FGL_APIENTRY fgl_func_glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
		fgl_api fgl_func_glBufferData* fgl_glBufferData;
#		define glBufferData fgl_glBufferData
		typedef void (FGL_APIENTRY fgl_func_glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
		fgl_api fgl_func_glBufferSubData* fgl_glBufferSubData;
#		define glBufferSubData fgl_glBufferSubData
		typedef void (FGL_APIENTRY fgl_func_glGetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
		fgl_api fgl_func_glGetBufferSubData* fgl_glGetBufferSubData;
#		define glGetBufferSubData fgl_glGetBufferSubData
		typedef void * (FGL_APIENTRY fgl_func_glMapBuffer)(GLenum target, GLenum access);
		fgl_api fgl_func_glMapBuffer* fgl_glMapBuffer;
#		define glMapBuffer fgl_glMapBuffer
		typedef GLboolean (FGL_APIENTRY fgl_func_glUnmapBuffer)(GLenum target);
		fgl_api fgl_func_glUnmapBuffer* fgl_glUnmapBuffer;
#		define glUnmapBuffer fgl_glUnmapBuffer
		typedef void (FGL_APIENTRY fgl_func_glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetBufferParameteriv* fgl_glGetBufferParameteriv;
#		define glGetBufferParameteriv fgl_glGetBufferParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGetBufferPointerv)(GLenum target, GLenum pname, void **params);
		fgl_api fgl_func_glGetBufferPointerv* fgl_glGetBufferPointerv;
#		define glGetBufferPointerv fgl_glGetBufferPointerv
#	endif // GL_VERSION_1_5

#	ifndef GL_VERSION_2_0
#		define GL_VERSION_2_0 1
		fgl_api bool isGL_VERSION_2_0;

		typedef char GLchar;

#		define GL_BLEND_EQUATION_RGB 0x8009
#		define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#		define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#		define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#		define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#		define GL_CURRENT_VERTEX_ATTRIB 0x8626
#		define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#		define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#		define GL_STENCIL_BACK_FUNC 0x8800
#		define GL_STENCIL_BACK_FAIL 0x8801
#		define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#		define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#		define GL_MAX_DRAW_BUFFERS 0x8824
#		define GL_DRAW_BUFFER0 0x8825
#		define GL_DRAW_BUFFER1 0x8826
#		define GL_DRAW_BUFFER2 0x8827
#		define GL_DRAW_BUFFER3 0x8828
#		define GL_DRAW_BUFFER4 0x8829
#		define GL_DRAW_BUFFER5 0x882A
#		define GL_DRAW_BUFFER6 0x882B
#		define GL_DRAW_BUFFER7 0x882C
#		define GL_DRAW_BUFFER8 0x882D
#		define GL_DRAW_BUFFER9 0x882E
#		define GL_DRAW_BUFFER10 0x882F
#		define GL_DRAW_BUFFER11 0x8830
#		define GL_DRAW_BUFFER12 0x8831
#		define GL_DRAW_BUFFER13 0x8832
#		define GL_DRAW_BUFFER14 0x8833
#		define GL_DRAW_BUFFER15 0x8834
#		define GL_BLEND_EQUATION_ALPHA 0x883D
#		define GL_MAX_VERTEX_ATTRIBS 0x8869
#		define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#		define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#		define GL_FRAGMENT_SHADER 0x8B30
#		define GL_VERTEX_SHADER 0x8B31
#		define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#		define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#		define GL_MAX_VARYING_FLOATS 0x8B4B
#		define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#		define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#		define GL_SHADER_TYPE 0x8B4F
#		define GL_FLOAT_VEC2 0x8B50
#		define GL_FLOAT_VEC3 0x8B51
#		define GL_FLOAT_VEC4 0x8B52
#		define GL_INT_VEC2 0x8B53
#		define GL_INT_VEC3 0x8B54
#		define GL_INT_VEC4 0x8B55
#		define GL_BOOL 0x8B56
#		define GL_BOOL_VEC2 0x8B57
#		define GL_BOOL_VEC3 0x8B58
#		define GL_BOOL_VEC4 0x8B59
#		define GL_FLOAT_MAT2 0x8B5A
#		define GL_FLOAT_MAT3 0x8B5B
#		define GL_FLOAT_MAT4 0x8B5C
#		define GL_SAMPLER_1D 0x8B5D
#		define GL_SAMPLER_2D 0x8B5E
#		define GL_SAMPLER_3D 0x8B5F
#		define GL_SAMPLER_CUBE 0x8B60
#		define GL_SAMPLER_1D_SHADOW 0x8B61
#		define GL_SAMPLER_2D_SHADOW 0x8B62
#		define GL_DELETE_STATUS 0x8B80
#		define GL_COMPILE_STATUS 0x8B81
#		define GL_LINK_STATUS 0x8B82
#		define GL_VALIDATE_STATUS 0x8B83
#		define GL_INFO_LOG_LENGTH 0x8B84
#		define GL_ATTACHED_SHADERS 0x8B85
#		define GL_ACTIVE_UNIFORMS 0x8B86
#		define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#		define GL_SHADER_SOURCE_LENGTH 0x8B88
#		define GL_ACTIVE_ATTRIBUTES 0x8B89
#		define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#		define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#		define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#		define GL_CURRENT_PROGRAM 0x8B8D
#		define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#		define GL_LOWER_LEFT 0x8CA1
#		define GL_UPPER_LEFT 0x8CA2
#		define GL_STENCIL_BACK_REF 0x8CA3
#		define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#		define GL_STENCIL_BACK_WRITEMASK 0x8CA5
#		define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#		define GL_POINT_SPRITE 0x8861
#		define GL_COORD_REPLACE 0x8862
#		define GL_MAX_TEXTURE_COORDS 0x8871

		typedef void (FGL_APIENTRY fgl_func_glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
		fgl_api fgl_func_glBlendEquationSeparate* fgl_glBlendEquationSeparate;
#		define glBlendEquationSeparate fgl_glBlendEquationSeparate
		typedef void (FGL_APIENTRY fgl_func_glDrawBuffers)(GLsizei n, const GLenum *bufs);
		fgl_api fgl_func_glDrawBuffers* fgl_glDrawBuffers;
#		define glDrawBuffers fgl_glDrawBuffers
		typedef void (FGL_APIENTRY fgl_func_glStencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
		fgl_api fgl_func_glStencilOpSeparate* fgl_glStencilOpSeparate;
#		define glStencilOpSeparate fgl_glStencilOpSeparate
		typedef void (FGL_APIENTRY fgl_func_glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
		fgl_api fgl_func_glStencilFuncSeparate* fgl_glStencilFuncSeparate;
#		define glStencilFuncSeparate fgl_glStencilFuncSeparate
		typedef void (FGL_APIENTRY fgl_func_glStencilMaskSeparate)(GLenum face, GLuint mask);
		fgl_api fgl_func_glStencilMaskSeparate* fgl_glStencilMaskSeparate;
#		define glStencilMaskSeparate fgl_glStencilMaskSeparate
		typedef void (FGL_APIENTRY fgl_func_glAttachShader)(GLuint program, GLuint shader);
		fgl_api fgl_func_glAttachShader* fgl_glAttachShader;
#		define glAttachShader fgl_glAttachShader
		typedef void (FGL_APIENTRY fgl_func_glBindAttribLocation)(GLuint program, GLuint index, const GLchar *name);
		fgl_api fgl_func_glBindAttribLocation* fgl_glBindAttribLocation;
#		define glBindAttribLocation fgl_glBindAttribLocation
		typedef void (FGL_APIENTRY fgl_func_glCompileShader)(GLuint shader);
		fgl_api fgl_func_glCompileShader* fgl_glCompileShader;
#		define glCompileShader fgl_glCompileShader
		typedef GLuint (FGL_APIENTRY fgl_func_glCreateProgram)(void);
		fgl_api fgl_func_glCreateProgram* fgl_glCreateProgram;
#		define glCreateProgram fgl_glCreateProgram
		typedef GLuint (FGL_APIENTRY fgl_func_glCreateShader)(GLenum type);
		fgl_api fgl_func_glCreateShader* fgl_glCreateShader;
#		define glCreateShader fgl_glCreateShader
		typedef void (FGL_APIENTRY fgl_func_glDeleteProgram)(GLuint program);
		fgl_api fgl_func_glDeleteProgram* fgl_glDeleteProgram;
#		define glDeleteProgram fgl_glDeleteProgram
		typedef void (FGL_APIENTRY fgl_func_glDeleteShader)(GLuint shader);
		fgl_api fgl_func_glDeleteShader* fgl_glDeleteShader;
#		define glDeleteShader fgl_glDeleteShader
		typedef void (FGL_APIENTRY fgl_func_glDetachShader)(GLuint program, GLuint shader);
		fgl_api fgl_func_glDetachShader* fgl_glDetachShader;
#		define glDetachShader fgl_glDetachShader
		typedef void (FGL_APIENTRY fgl_func_glDisableVertexAttribArray)(GLuint index);
		fgl_api fgl_func_glDisableVertexAttribArray* fgl_glDisableVertexAttribArray;
#		define glDisableVertexAttribArray fgl_glDisableVertexAttribArray
		typedef void (FGL_APIENTRY fgl_func_glEnableVertexAttribArray)(GLuint index);
		fgl_api fgl_func_glEnableVertexAttribArray* fgl_glEnableVertexAttribArray;
#		define glEnableVertexAttribArray fgl_glEnableVertexAttribArray
		typedef void (FGL_APIENTRY fgl_func_glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
		fgl_api fgl_func_glGetActiveAttrib* fgl_glGetActiveAttrib;
#		define glGetActiveAttrib fgl_glGetActiveAttrib
		typedef void (FGL_APIENTRY fgl_func_glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
		fgl_api fgl_func_glGetActiveUniform* fgl_glGetActiveUniform;
#		define glGetActiveUniform fgl_glGetActiveUniform
		typedef void (FGL_APIENTRY fgl_func_glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
		fgl_api fgl_func_glGetAttachedShaders* fgl_glGetAttachedShaders;
#		define glGetAttachedShaders fgl_glGetAttachedShaders
		typedef GLint (FGL_APIENTRY fgl_func_glGetAttribLocation)(GLuint program, const GLchar *name);
		fgl_api fgl_func_glGetAttribLocation* fgl_glGetAttribLocation;
#		define glGetAttribLocation fgl_glGetAttribLocation
		typedef void (FGL_APIENTRY fgl_func_glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetProgramiv* fgl_glGetProgramiv;
#		define glGetProgramiv fgl_glGetProgramiv
		typedef void (FGL_APIENTRY fgl_func_glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
		fgl_api fgl_func_glGetProgramInfoLog* fgl_glGetProgramInfoLog;
#		define glGetProgramInfoLog fgl_glGetProgramInfoLog
		typedef void (FGL_APIENTRY fgl_func_glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetShaderiv* fgl_glGetShaderiv;
#		define glGetShaderiv fgl_glGetShaderiv
		typedef void (FGL_APIENTRY fgl_func_glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
		fgl_api fgl_func_glGetShaderInfoLog* fgl_glGetShaderInfoLog;
#		define glGetShaderInfoLog fgl_glGetShaderInfoLog
		typedef void (FGL_APIENTRY fgl_func_glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
		fgl_api fgl_func_glGetShaderSource* fgl_glGetShaderSource;
#		define glGetShaderSource fgl_glGetShaderSource
		typedef GLint (FGL_APIENTRY fgl_func_glGetUniformLocation)(GLuint program, const GLchar *name);
		fgl_api fgl_func_glGetUniformLocation* fgl_glGetUniformLocation;
#		define glGetUniformLocation fgl_glGetUniformLocation
		typedef void (FGL_APIENTRY fgl_func_glGetUniformfv)(GLuint program, GLint location, GLfloat *params);
		fgl_api fgl_func_glGetUniformfv* fgl_glGetUniformfv;
#		define glGetUniformfv fgl_glGetUniformfv
		typedef void (FGL_APIENTRY fgl_func_glGetUniformiv)(GLuint program, GLint location, GLint *params);
		fgl_api fgl_func_glGetUniformiv* fgl_glGetUniformiv;
#		define glGetUniformiv fgl_glGetUniformiv
		typedef void (FGL_APIENTRY fgl_func_glGetVertexAttribdv)(GLuint index, GLenum pname, GLdouble *params);
		fgl_api fgl_func_glGetVertexAttribdv* fgl_glGetVertexAttribdv;
#		define glGetVertexAttribdv fgl_glGetVertexAttribdv
		typedef void (FGL_APIENTRY fgl_func_glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetVertexAttribfv* fgl_glGetVertexAttribfv;
#		define glGetVertexAttribfv fgl_glGetVertexAttribfv
		typedef void (FGL_APIENTRY fgl_func_glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetVertexAttribiv* fgl_glGetVertexAttribiv;
#		define glGetVertexAttribiv fgl_glGetVertexAttribiv
		typedef void (FGL_APIENTRY fgl_func_glGetVertexAttribPointerv)(GLuint index, GLenum pname, void **pointer);
		fgl_api fgl_func_glGetVertexAttribPointerv* fgl_glGetVertexAttribPointerv;
#		define glGetVertexAttribPointerv fgl_glGetVertexAttribPointerv
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsProgram)(GLuint program);
		fgl_api fgl_func_glIsProgram* fgl_glIsProgram;
#		define glIsProgram fgl_glIsProgram
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsShader)(GLuint shader);
		fgl_api fgl_func_glIsShader* fgl_glIsShader;
#		define glIsShader fgl_glIsShader
		typedef void (FGL_APIENTRY fgl_func_glLinkProgram)(GLuint program);
		fgl_api fgl_func_glLinkProgram* fgl_glLinkProgram;
#		define glLinkProgram fgl_glLinkProgram
		typedef void (FGL_APIENTRY fgl_func_glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
		fgl_api fgl_func_glShaderSource* fgl_glShaderSource;
#		define glShaderSource fgl_glShaderSource
		typedef void (FGL_APIENTRY fgl_func_glUseProgram)(GLuint program);
		fgl_api fgl_func_glUseProgram* fgl_glUseProgram;
#		define glUseProgram fgl_glUseProgram
		typedef void (FGL_APIENTRY fgl_func_glUniform1f)(GLint location, GLfloat v0);
		fgl_api fgl_func_glUniform1f* fgl_glUniform1f;
#		define glUniform1f fgl_glUniform1f
		typedef void (FGL_APIENTRY fgl_func_glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
		fgl_api fgl_func_glUniform2f* fgl_glUniform2f;
#		define glUniform2f fgl_glUniform2f
		typedef void (FGL_APIENTRY fgl_func_glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
		fgl_api fgl_func_glUniform3f* fgl_glUniform3f;
#		define glUniform3f fgl_glUniform3f
		typedef void (FGL_APIENTRY fgl_func_glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
		fgl_api fgl_func_glUniform4f* fgl_glUniform4f;
#		define glUniform4f fgl_glUniform4f
		typedef void (FGL_APIENTRY fgl_func_glUniform1i)(GLint location, GLint v0);
		fgl_api fgl_func_glUniform1i* fgl_glUniform1i;
#		define glUniform1i fgl_glUniform1i
		typedef void (FGL_APIENTRY fgl_func_glUniform2i)(GLint location, GLint v0, GLint v1);
		fgl_api fgl_func_glUniform2i* fgl_glUniform2i;
#		define glUniform2i fgl_glUniform2i
		typedef void (FGL_APIENTRY fgl_func_glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
		fgl_api fgl_func_glUniform3i* fgl_glUniform3i;
#		define glUniform3i fgl_glUniform3i
		typedef void (FGL_APIENTRY fgl_func_glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
		fgl_api fgl_func_glUniform4i* fgl_glUniform4i;
#		define glUniform4i fgl_glUniform4i
		typedef void (FGL_APIENTRY fgl_func_glUniform1fv)(GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glUniform1fv* fgl_glUniform1fv;
#		define glUniform1fv fgl_glUniform1fv
		typedef void (FGL_APIENTRY fgl_func_glUniform2fv)(GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glUniform2fv* fgl_glUniform2fv;
#		define glUniform2fv fgl_glUniform2fv
		typedef void (FGL_APIENTRY fgl_func_glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glUniform3fv* fgl_glUniform3fv;
#		define glUniform3fv fgl_glUniform3fv
		typedef void (FGL_APIENTRY fgl_func_glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glUniform4fv* fgl_glUniform4fv;
#		define glUniform4fv fgl_glUniform4fv
		typedef void (FGL_APIENTRY fgl_func_glUniform1iv)(GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glUniform1iv* fgl_glUniform1iv;
#		define glUniform1iv fgl_glUniform1iv
		typedef void (FGL_APIENTRY fgl_func_glUniform2iv)(GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glUniform2iv* fgl_glUniform2iv;
#		define glUniform2iv fgl_glUniform2iv
		typedef void (FGL_APIENTRY fgl_func_glUniform3iv)(GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glUniform3iv* fgl_glUniform3iv;
#		define glUniform3iv fgl_glUniform3iv
		typedef void (FGL_APIENTRY fgl_func_glUniform4iv)(GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glUniform4iv* fgl_glUniform4iv;
#		define glUniform4iv fgl_glUniform4iv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix2fv* fgl_glUniformMatrix2fv;
#		define glUniformMatrix2fv fgl_glUniformMatrix2fv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix3fv* fgl_glUniformMatrix3fv;
#		define glUniformMatrix3fv fgl_glUniformMatrix3fv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix4fv* fgl_glUniformMatrix4fv;
#		define glUniformMatrix4fv fgl_glUniformMatrix4fv
		typedef void (FGL_APIENTRY fgl_func_glValidateProgram)(GLuint program);
		fgl_api fgl_func_glValidateProgram* fgl_glValidateProgram;
#		define glValidateProgram fgl_glValidateProgram
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib1d)(GLuint index, GLdouble x);
		fgl_api fgl_func_glVertexAttrib1d* fgl_glVertexAttrib1d;
#		define glVertexAttrib1d fgl_glVertexAttrib1d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib1dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttrib1dv* fgl_glVertexAttrib1dv;
#		define glVertexAttrib1dv fgl_glVertexAttrib1dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib1f)(GLuint index, GLfloat x);
		fgl_api fgl_func_glVertexAttrib1f* fgl_glVertexAttrib1f;
#		define glVertexAttrib1f fgl_glVertexAttrib1f
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib1fv)(GLuint index, const GLfloat *v);
		fgl_api fgl_func_glVertexAttrib1fv* fgl_glVertexAttrib1fv;
#		define glVertexAttrib1fv fgl_glVertexAttrib1fv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib1s)(GLuint index, GLshort x);
		fgl_api fgl_func_glVertexAttrib1s* fgl_glVertexAttrib1s;
#		define glVertexAttrib1s fgl_glVertexAttrib1s
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib1sv)(GLuint index, const GLshort *v);
		fgl_api fgl_func_glVertexAttrib1sv* fgl_glVertexAttrib1sv;
#		define glVertexAttrib1sv fgl_glVertexAttrib1sv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib2d)(GLuint index, GLdouble x, GLdouble y);
		fgl_api fgl_func_glVertexAttrib2d* fgl_glVertexAttrib2d;
#		define glVertexAttrib2d fgl_glVertexAttrib2d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib2dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttrib2dv* fgl_glVertexAttrib2dv;
#		define glVertexAttrib2dv fgl_glVertexAttrib2dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
		fgl_api fgl_func_glVertexAttrib2f* fgl_glVertexAttrib2f;
#		define glVertexAttrib2f fgl_glVertexAttrib2f
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib2fv)(GLuint index, const GLfloat *v);
		fgl_api fgl_func_glVertexAttrib2fv* fgl_glVertexAttrib2fv;
#		define glVertexAttrib2fv fgl_glVertexAttrib2fv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib2s)(GLuint index, GLshort x, GLshort y);
		fgl_api fgl_func_glVertexAttrib2s* fgl_glVertexAttrib2s;
#		define glVertexAttrib2s fgl_glVertexAttrib2s
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib2sv)(GLuint index, const GLshort *v);
		fgl_api fgl_func_glVertexAttrib2sv* fgl_glVertexAttrib2sv;
#		define glVertexAttrib2sv fgl_glVertexAttrib2sv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glVertexAttrib3d* fgl_glVertexAttrib3d;
#		define glVertexAttrib3d fgl_glVertexAttrib3d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib3dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttrib3dv* fgl_glVertexAttrib3dv;
#		define glVertexAttrib3dv fgl_glVertexAttrib3dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
		fgl_api fgl_func_glVertexAttrib3f* fgl_glVertexAttrib3f;
#		define glVertexAttrib3f fgl_glVertexAttrib3f
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib3fv)(GLuint index, const GLfloat *v);
		fgl_api fgl_func_glVertexAttrib3fv* fgl_glVertexAttrib3fv;
#		define glVertexAttrib3fv fgl_glVertexAttrib3fv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z);
		fgl_api fgl_func_glVertexAttrib3s* fgl_glVertexAttrib3s;
#		define glVertexAttrib3s fgl_glVertexAttrib3s
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib3sv)(GLuint index, const GLshort *v);
		fgl_api fgl_func_glVertexAttrib3sv* fgl_glVertexAttrib3sv;
#		define glVertexAttrib3sv fgl_glVertexAttrib3sv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4Nbv)(GLuint index, const GLbyte *v);
		fgl_api fgl_func_glVertexAttrib4Nbv* fgl_glVertexAttrib4Nbv;
#		define glVertexAttrib4Nbv fgl_glVertexAttrib4Nbv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4Niv)(GLuint index, const GLint *v);
		fgl_api fgl_func_glVertexAttrib4Niv* fgl_glVertexAttrib4Niv;
#		define glVertexAttrib4Niv fgl_glVertexAttrib4Niv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4Nsv)(GLuint index, const GLshort *v);
		fgl_api fgl_func_glVertexAttrib4Nsv* fgl_glVertexAttrib4Nsv;
#		define glVertexAttrib4Nsv fgl_glVertexAttrib4Nsv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
		fgl_api fgl_func_glVertexAttrib4Nub* fgl_glVertexAttrib4Nub;
#		define glVertexAttrib4Nub fgl_glVertexAttrib4Nub
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4Nubv)(GLuint index, const GLubyte *v);
		fgl_api fgl_func_glVertexAttrib4Nubv* fgl_glVertexAttrib4Nubv;
#		define glVertexAttrib4Nubv fgl_glVertexAttrib4Nubv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4Nuiv)(GLuint index, const GLuint *v);
		fgl_api fgl_func_glVertexAttrib4Nuiv* fgl_glVertexAttrib4Nuiv;
#		define glVertexAttrib4Nuiv fgl_glVertexAttrib4Nuiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4Nusv)(GLuint index, const GLushort *v);
		fgl_api fgl_func_glVertexAttrib4Nusv* fgl_glVertexAttrib4Nusv;
#		define glVertexAttrib4Nusv fgl_glVertexAttrib4Nusv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4bv)(GLuint index, const GLbyte *v);
		fgl_api fgl_func_glVertexAttrib4bv* fgl_glVertexAttrib4bv;
#		define glVertexAttrib4bv fgl_glVertexAttrib4bv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		fgl_api fgl_func_glVertexAttrib4d* fgl_glVertexAttrib4d;
#		define glVertexAttrib4d fgl_glVertexAttrib4d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttrib4dv* fgl_glVertexAttrib4dv;
#		define glVertexAttrib4dv fgl_glVertexAttrib4dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		fgl_api fgl_func_glVertexAttrib4f* fgl_glVertexAttrib4f;
#		define glVertexAttrib4f fgl_glVertexAttrib4f
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4fv)(GLuint index, const GLfloat *v);
		fgl_api fgl_func_glVertexAttrib4fv* fgl_glVertexAttrib4fv;
#		define glVertexAttrib4fv fgl_glVertexAttrib4fv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4iv)(GLuint index, const GLint *v);
		fgl_api fgl_func_glVertexAttrib4iv* fgl_glVertexAttrib4iv;
#		define glVertexAttrib4iv fgl_glVertexAttrib4iv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
		fgl_api fgl_func_glVertexAttrib4s* fgl_glVertexAttrib4s;
#		define glVertexAttrib4s fgl_glVertexAttrib4s
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4sv)(GLuint index, const GLshort *v);
		fgl_api fgl_func_glVertexAttrib4sv* fgl_glVertexAttrib4sv;
#		define glVertexAttrib4sv fgl_glVertexAttrib4sv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4ubv)(GLuint index, const GLubyte *v);
		fgl_api fgl_func_glVertexAttrib4ubv* fgl_glVertexAttrib4ubv;
#		define glVertexAttrib4ubv fgl_glVertexAttrib4ubv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4uiv)(GLuint index, const GLuint *v);
		fgl_api fgl_func_glVertexAttrib4uiv* fgl_glVertexAttrib4uiv;
#		define glVertexAttrib4uiv fgl_glVertexAttrib4uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttrib4usv)(GLuint index, const GLushort *v);
		fgl_api fgl_func_glVertexAttrib4usv* fgl_glVertexAttrib4usv;
#		define glVertexAttrib4usv fgl_glVertexAttrib4usv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
		fgl_api fgl_func_glVertexAttribPointer* fgl_glVertexAttribPointer;
#		define glVertexAttribPointer fgl_glVertexAttribPointer
#	endif // GL_VERSION_2_0

#	ifndef GL_VERSION_2_1
#		define GL_VERSION_2_1 1
		fgl_api bool isGL_VERSION_2_1;

#		define GL_PIXEL_PACK_BUFFER 0x88EB
#		define GL_PIXEL_UNPACK_BUFFER 0x88EC
#		define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#		define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#		define GL_FLOAT_MAT2x3 0x8B65
#		define GL_FLOAT_MAT2x4 0x8B66
#		define GL_FLOAT_MAT3x2 0x8B67
#		define GL_FLOAT_MAT3x4 0x8B68
#		define GL_FLOAT_MAT4x2 0x8B69
#		define GL_FLOAT_MAT4x3 0x8B6A
#		define GL_SRGB 0x8C40
#		define GL_SRGB8 0x8C41
#		define GL_SRGB_ALPHA 0x8C42
#		define GL_SRGB8_ALPHA8 0x8C43
#		define GL_COMPRESSED_SRGB 0x8C48
#		define GL_COMPRESSED_SRGB_ALPHA 0x8C49
#		define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#		define GL_SLUMINANCE_ALPHA 0x8C44
#		define GL_SLUMINANCE8_ALPHA8 0x8C45
#		define GL_SLUMINANCE 0x8C46
#		define GL_SLUMINANCE8 0x8C47
#		define GL_COMPRESSED_SLUMINANCE 0x8C4A
#		define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B

		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix2x3fv* fgl_glUniformMatrix2x3fv;
#		define glUniformMatrix2x3fv fgl_glUniformMatrix2x3fv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix3x2fv* fgl_glUniformMatrix3x2fv;
#		define glUniformMatrix3x2fv fgl_glUniformMatrix3x2fv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix2x4fv* fgl_glUniformMatrix2x4fv;
#		define glUniformMatrix2x4fv fgl_glUniformMatrix2x4fv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix4x2fv* fgl_glUniformMatrix4x2fv;
#		define glUniformMatrix4x2fv fgl_glUniformMatrix4x2fv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix3x4fv* fgl_glUniformMatrix3x4fv;
#		define glUniformMatrix3x4fv fgl_glUniformMatrix3x4fv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glUniformMatrix4x3fv* fgl_glUniformMatrix4x3fv;
#		define glUniformMatrix4x3fv fgl_glUniformMatrix4x3fv
#	endif // GL_VERSION_2_1

#	ifndef GL_VERSION_3_0
#		define GL_VERSION_3_0 1
		fgl_api bool isGL_VERSION_3_0;

#		define GL_COMPARE_REF_TO_TEXTURE 0x884E
#		define GL_CLIP_DISTANCE0 0x3000
#		define GL_CLIP_DISTANCE1 0x3001
#		define GL_CLIP_DISTANCE2 0x3002
#		define GL_CLIP_DISTANCE3 0x3003
#		define GL_CLIP_DISTANCE4 0x3004
#		define GL_CLIP_DISTANCE5 0x3005
#		define GL_CLIP_DISTANCE6 0x3006
#		define GL_CLIP_DISTANCE7 0x3007
#		define GL_MAX_CLIP_DISTANCES 0x0D32
#		define GL_MAJOR_VERSION 0x821B
#		define GL_MINOR_VERSION 0x821C
#		define GL_NUM_EXTENSIONS 0x821D
#		define GL_CONTEXT_FLAGS 0x821E
#		define GL_COMPRESSED_RED 0x8225
#		define GL_COMPRESSED_RG 0x8226
#		define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#		define GL_RGBA32F 0x8814
#		define GL_RGB32F 0x8815
#		define GL_RGBA16F 0x881A
#		define GL_RGB16F 0x881B
#		define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
#		define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#		define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#		define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#		define GL_CLAMP_READ_COLOR 0x891C
#		define GL_FIXED_ONLY 0x891D
#		define GL_MAX_VARYING_COMPONENTS 0x8B4B
#		define GL_TEXTURE_1D_ARRAY 0x8C18
#		define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
#		define GL_TEXTURE_2D_ARRAY 0x8C1A
#		define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
#		define GL_TEXTURE_BINDING_1D_ARRAY 0x8C1C
#		define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
#		define GL_R11F_G11F_B10F 0x8C3A
#		define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#		define GL_RGB9_E5 0x8C3D
#		define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#		define GL_TEXTURE_SHARED_SIZE 0x8C3F
#		define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#		define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#		define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#		define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#		define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#		define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#		define GL_PRIMITIVES_GENERATED 0x8C87
#		define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#		define GL_RASTERIZER_DISCARD 0x8C89
#		define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#		define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#		define GL_INTERLEAVED_ATTRIBS 0x8C8C
#		define GL_SEPARATE_ATTRIBS 0x8C8D
#		define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#		define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#		define GL_RGBA32UI 0x8D70
#		define GL_RGB32UI 0x8D71
#		define GL_RGBA16UI 0x8D76
#		define GL_RGB16UI 0x8D77
#		define GL_RGBA8UI 0x8D7C
#		define GL_RGB8UI 0x8D7D
#		define GL_RGBA32I 0x8D82
#		define GL_RGB32I 0x8D83
#		define GL_RGBA16I 0x8D88
#		define GL_RGB16I 0x8D89
#		define GL_RGBA8I 0x8D8E
#		define GL_RGB8I 0x8D8F
#		define GL_RED_INTEGER 0x8D94
#		define GL_GREEN_INTEGER 0x8D95
#		define GL_BLUE_INTEGER 0x8D96
#		define GL_RGB_INTEGER 0x8D98
#		define GL_RGBA_INTEGER 0x8D99
#		define GL_BGR_INTEGER 0x8D9A
#		define GL_BGRA_INTEGER 0x8D9B
#		define GL_SAMPLER_1D_ARRAY 0x8DC0
#		define GL_SAMPLER_2D_ARRAY 0x8DC1
#		define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#		define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
#		define GL_SAMPLER_CUBE_SHADOW 0x8DC5
#		define GL_UNSIGNED_INT_VEC2 0x8DC6
#		define GL_UNSIGNED_INT_VEC3 0x8DC7
#		define GL_UNSIGNED_INT_VEC4 0x8DC8
#		define GL_INT_SAMPLER_1D 0x8DC9
#		define GL_INT_SAMPLER_2D 0x8DCA
#		define GL_INT_SAMPLER_3D 0x8DCB
#		define GL_INT_SAMPLER_CUBE 0x8DCC
#		define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#		define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
#		define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#		define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
#		define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
#		define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
#		define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#		define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
#		define GL_QUERY_WAIT 0x8E13
#		define GL_QUERY_NO_WAIT 0x8E14
#		define GL_QUERY_BY_REGION_WAIT 0x8E15
#		define GL_QUERY_BY_REGION_NO_WAIT 0x8E16
#		define GL_BUFFER_ACCESS_FLAGS 0x911F
#		define GL_BUFFER_MAP_LENGTH 0x9120
#		define GL_BUFFER_MAP_OFFSET 0x9121
#		define GL_DEPTH_COMPONENT32F 0x8CAC
#		define GL_DEPTH32F_STENCIL8 0x8CAD
#		define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#		define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#		define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#		define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#		define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#		define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#		define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#		define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#		define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#		define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#		define GL_FRAMEBUFFER_DEFAULT 0x8218
#		define GL_FRAMEBUFFER_UNDEFINED 0x8219
#		define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#		define GL_MAX_RENDERBUFFER_SIZE 0x84E8
#		define GL_DEPTH_STENCIL 0x84F9
#		define GL_UNSIGNED_INT_24_8 0x84FA
#		define GL_DEPTH24_STENCIL8 0x88F0
#		define GL_TEXTURE_STENCIL_SIZE 0x88F1
#		define GL_TEXTURE_RED_TYPE 0x8C10
#		define GL_TEXTURE_GREEN_TYPE 0x8C11
#		define GL_TEXTURE_BLUE_TYPE 0x8C12
#		define GL_TEXTURE_ALPHA_TYPE 0x8C13
#		define GL_TEXTURE_DEPTH_TYPE 0x8C16
#		define GL_UNSIGNED_NORMALIZED 0x8C17
#		define GL_FRAMEBUFFER_BINDING 0x8CA6
#		define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#		define GL_RENDERBUFFER_BINDING 0x8CA7
#		define GL_READ_FRAMEBUFFER 0x8CA8
#		define GL_DRAW_FRAMEBUFFER 0x8CA9
#		define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#		define GL_RENDERBUFFER_SAMPLES 0x8CAB
#		define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#		define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#		define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#		define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#		define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#		define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#		define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#		define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#		define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#		define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#		define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#		define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#		define GL_COLOR_ATTACHMENT0 0x8CE0
#		define GL_COLOR_ATTACHMENT1 0x8CE1
#		define GL_COLOR_ATTACHMENT2 0x8CE2
#		define GL_COLOR_ATTACHMENT3 0x8CE3
#		define GL_COLOR_ATTACHMENT4 0x8CE4
#		define GL_COLOR_ATTACHMENT5 0x8CE5
#		define GL_COLOR_ATTACHMENT6 0x8CE6
#		define GL_COLOR_ATTACHMENT7 0x8CE7
#		define GL_COLOR_ATTACHMENT8 0x8CE8
#		define GL_COLOR_ATTACHMENT9 0x8CE9
#		define GL_COLOR_ATTACHMENT10 0x8CEA
#		define GL_COLOR_ATTACHMENT11 0x8CEB
#		define GL_COLOR_ATTACHMENT12 0x8CEC
#		define GL_COLOR_ATTACHMENT13 0x8CED
#		define GL_COLOR_ATTACHMENT14 0x8CEE
#		define GL_COLOR_ATTACHMENT15 0x8CEF
#		define GL_COLOR_ATTACHMENT16 0x8CF0
#		define GL_COLOR_ATTACHMENT17 0x8CF1
#		define GL_COLOR_ATTACHMENT18 0x8CF2
#		define GL_COLOR_ATTACHMENT19 0x8CF3
#		define GL_COLOR_ATTACHMENT20 0x8CF4
#		define GL_COLOR_ATTACHMENT21 0x8CF5
#		define GL_COLOR_ATTACHMENT22 0x8CF6
#		define GL_COLOR_ATTACHMENT23 0x8CF7
#		define GL_COLOR_ATTACHMENT24 0x8CF8
#		define GL_COLOR_ATTACHMENT25 0x8CF9
#		define GL_COLOR_ATTACHMENT26 0x8CFA
#		define GL_COLOR_ATTACHMENT27 0x8CFB
#		define GL_COLOR_ATTACHMENT28 0x8CFC
#		define GL_COLOR_ATTACHMENT29 0x8CFD
#		define GL_COLOR_ATTACHMENT30 0x8CFE
#		define GL_COLOR_ATTACHMENT31 0x8CFF
#		define GL_DEPTH_ATTACHMENT 0x8D00
#		define GL_STENCIL_ATTACHMENT 0x8D20
#		define GL_FRAMEBUFFER 0x8D40
#		define GL_RENDERBUFFER 0x8D41
#		define GL_RENDERBUFFER_WIDTH 0x8D42
#		define GL_RENDERBUFFER_HEIGHT 0x8D43
#		define GL_RENDERBUFFER_INTERNAL_FORMAT 0x8D44
#		define GL_STENCIL_INDEX1 0x8D46
#		define GL_STENCIL_INDEX4 0x8D47
#		define GL_STENCIL_INDEX8 0x8D48
#		define GL_STENCIL_INDEX16 0x8D49
#		define GL_RENDERBUFFER_RED_SIZE 0x8D50
#		define GL_RENDERBUFFER_GREEN_SIZE 0x8D51
#		define GL_RENDERBUFFER_BLUE_SIZE 0x8D52
#		define GL_RENDERBUFFER_ALPHA_SIZE 0x8D53
#		define GL_RENDERBUFFER_DEPTH_SIZE 0x8D54
#		define GL_RENDERBUFFER_STENCIL_SIZE 0x8D55
#		define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#		define GL_MAX_SAMPLES 0x8D57
#		define GL_INDEX 0x8222
#		define GL_TEXTURE_LUMINANCE_TYPE 0x8C14
#		define GL_TEXTURE_INTENSITY_TYPE 0x8C15
#		define GL_FRAMEBUFFER_SRGB 0x8DB9
#		define GL_HALF_FLOAT 0x140B
#		define GL_MAP_READ_BIT 0x0001
#		define GL_MAP_WRITE_BIT 0x0002
#		define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#		define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#		define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#		define GL_MAP_UNSYNCHRONIZED_BIT 0x0020
#		define GL_COMPRESSED_RED_RGTC1 0x8DBB
#		define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#		define GL_COMPRESSED_RG_RGTC2 0x8DBD
#		define GL_COMPRESSED_SIGNED_RG_RGTC2 0x8DBE
#		define GL_RG 0x8227
#		define GL_RG_INTEGER 0x8228
#		define GL_R8 0x8229
#		define GL_R16 0x822A
#		define GL_RG8 0x822B
#		define GL_RG16 0x822C
#		define GL_R16F 0x822D
#		define GL_R32F 0x822E
#		define GL_RG16F 0x822F
#		define GL_RG32F 0x8230
#		define GL_R8I 0x8231
#		define GL_R8UI 0x8232
#		define GL_R16I 0x8233
#		define GL_R16UI 0x8234
#		define GL_R32I 0x8235
#		define GL_R32UI 0x8236
#		define GL_RG8I 0x8237
#		define GL_RG8UI 0x8238
#		define GL_RG16I 0x8239
#		define GL_RG16UI 0x823A
#		define GL_RG32I 0x823B
#		define GL_RG32UI 0x823C
#		define GL_VERTEX_ARRAY_BINDING 0x85B5
#		define GL_CLAMP_VERTEX_COLOR 0x891A
#		define GL_CLAMP_FRAGMENT_COLOR 0x891B
#		define GL_ALPHA_INTEGER 0x8D97

		typedef void (FGL_APIENTRY fgl_func_glColorMaski)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
		fgl_api fgl_func_glColorMaski* fgl_glColorMaski;
#		define glColorMaski fgl_glColorMaski
		typedef void (FGL_APIENTRY fgl_func_glGetBooleani_v)(GLenum target, GLuint index, GLboolean *data);
		fgl_api fgl_func_glGetBooleani_v* fgl_glGetBooleani_v;
#		define glGetBooleani_v fgl_glGetBooleani_v
		typedef void (FGL_APIENTRY fgl_func_glGetIntegeri_v)(GLenum target, GLuint index, GLint *data);
		fgl_api fgl_func_glGetIntegeri_v* fgl_glGetIntegeri_v;
#		define glGetIntegeri_v fgl_glGetIntegeri_v
		typedef void (FGL_APIENTRY fgl_func_glEnablei)(GLenum target, GLuint index);
		fgl_api fgl_func_glEnablei* fgl_glEnablei;
#		define glEnablei fgl_glEnablei
		typedef void (FGL_APIENTRY fgl_func_glDisablei)(GLenum target, GLuint index);
		fgl_api fgl_func_glDisablei* fgl_glDisablei;
#		define glDisablei fgl_glDisablei
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsEnabledi)(GLenum target, GLuint index);
		fgl_api fgl_func_glIsEnabledi* fgl_glIsEnabledi;
#		define glIsEnabledi fgl_glIsEnabledi
		typedef void (FGL_APIENTRY fgl_func_glBeginTransformFeedback)(GLenum primitiveMode);
		fgl_api fgl_func_glBeginTransformFeedback* fgl_glBeginTransformFeedback;
#		define glBeginTransformFeedback fgl_glBeginTransformFeedback
		typedef void (FGL_APIENTRY fgl_func_glEndTransformFeedback)(void);
		fgl_api fgl_func_glEndTransformFeedback* fgl_glEndTransformFeedback;
#		define glEndTransformFeedback fgl_glEndTransformFeedback
		typedef void (FGL_APIENTRY fgl_func_glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
		fgl_api fgl_func_glBindBufferRange* fgl_glBindBufferRange;
#		define glBindBufferRange fgl_glBindBufferRange
		typedef void (FGL_APIENTRY fgl_func_glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
		fgl_api fgl_func_glBindBufferBase* fgl_glBindBufferBase;
#		define glBindBufferBase fgl_glBindBufferBase
		typedef void (FGL_APIENTRY fgl_func_glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
		fgl_api fgl_func_glTransformFeedbackVaryings* fgl_glTransformFeedbackVaryings;
#		define glTransformFeedbackVaryings fgl_glTransformFeedbackVaryings
		typedef void (FGL_APIENTRY fgl_func_glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
		fgl_api fgl_func_glGetTransformFeedbackVarying* fgl_glGetTransformFeedbackVarying;
#		define glGetTransformFeedbackVarying fgl_glGetTransformFeedbackVarying
		typedef void (FGL_APIENTRY fgl_func_glClampColor)(GLenum target, GLenum clamp);
		fgl_api fgl_func_glClampColor* fgl_glClampColor;
#		define glClampColor fgl_glClampColor
		typedef void (FGL_APIENTRY fgl_func_glBeginConditionalRender)(GLuint id, GLenum mode);
		fgl_api fgl_func_glBeginConditionalRender* fgl_glBeginConditionalRender;
#		define glBeginConditionalRender fgl_glBeginConditionalRender
		typedef void (FGL_APIENTRY fgl_func_glEndConditionalRender)(void);
		fgl_api fgl_func_glEndConditionalRender* fgl_glEndConditionalRender;
#		define glEndConditionalRender fgl_glEndConditionalRender
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
		fgl_api fgl_func_glVertexAttribIPointer* fgl_glVertexAttribIPointer;
#		define glVertexAttribIPointer fgl_glVertexAttribIPointer
		typedef void (FGL_APIENTRY fgl_func_glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetVertexAttribIiv* fgl_glGetVertexAttribIiv;
#		define glGetVertexAttribIiv fgl_glGetVertexAttribIiv
		typedef void (FGL_APIENTRY fgl_func_glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params);
		fgl_api fgl_func_glGetVertexAttribIuiv* fgl_glGetVertexAttribIuiv;
#		define glGetVertexAttribIuiv fgl_glGetVertexAttribIuiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI1i)(GLuint index, GLint x);
		fgl_api fgl_func_glVertexAttribI1i* fgl_glVertexAttribI1i;
#		define glVertexAttribI1i fgl_glVertexAttribI1i
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI2i)(GLuint index, GLint x, GLint y);
		fgl_api fgl_func_glVertexAttribI2i* fgl_glVertexAttribI2i;
#		define glVertexAttribI2i fgl_glVertexAttribI2i
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z);
		fgl_api fgl_func_glVertexAttribI3i* fgl_glVertexAttribI3i;
#		define glVertexAttribI3i fgl_glVertexAttribI3i
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
		fgl_api fgl_func_glVertexAttribI4i* fgl_glVertexAttribI4i;
#		define glVertexAttribI4i fgl_glVertexAttribI4i
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI1ui)(GLuint index, GLuint x);
		fgl_api fgl_func_glVertexAttribI1ui* fgl_glVertexAttribI1ui;
#		define glVertexAttribI1ui fgl_glVertexAttribI1ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI2ui)(GLuint index, GLuint x, GLuint y);
		fgl_api fgl_func_glVertexAttribI2ui* fgl_glVertexAttribI2ui;
#		define glVertexAttribI2ui fgl_glVertexAttribI2ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z);
		fgl_api fgl_func_glVertexAttribI3ui* fgl_glVertexAttribI3ui;
#		define glVertexAttribI3ui fgl_glVertexAttribI3ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
		fgl_api fgl_func_glVertexAttribI4ui* fgl_glVertexAttribI4ui;
#		define glVertexAttribI4ui fgl_glVertexAttribI4ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI1iv)(GLuint index, const GLint *v);
		fgl_api fgl_func_glVertexAttribI1iv* fgl_glVertexAttribI1iv;
#		define glVertexAttribI1iv fgl_glVertexAttribI1iv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI2iv)(GLuint index, const GLint *v);
		fgl_api fgl_func_glVertexAttribI2iv* fgl_glVertexAttribI2iv;
#		define glVertexAttribI2iv fgl_glVertexAttribI2iv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI3iv)(GLuint index, const GLint *v);
		fgl_api fgl_func_glVertexAttribI3iv* fgl_glVertexAttribI3iv;
#		define glVertexAttribI3iv fgl_glVertexAttribI3iv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4iv)(GLuint index, const GLint *v);
		fgl_api fgl_func_glVertexAttribI4iv* fgl_glVertexAttribI4iv;
#		define glVertexAttribI4iv fgl_glVertexAttribI4iv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI1uiv)(GLuint index, const GLuint *v);
		fgl_api fgl_func_glVertexAttribI1uiv* fgl_glVertexAttribI1uiv;
#		define glVertexAttribI1uiv fgl_glVertexAttribI1uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI2uiv)(GLuint index, const GLuint *v);
		fgl_api fgl_func_glVertexAttribI2uiv* fgl_glVertexAttribI2uiv;
#		define glVertexAttribI2uiv fgl_glVertexAttribI2uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI3uiv)(GLuint index, const GLuint *v);
		fgl_api fgl_func_glVertexAttribI3uiv* fgl_glVertexAttribI3uiv;
#		define glVertexAttribI3uiv fgl_glVertexAttribI3uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4uiv)(GLuint index, const GLuint *v);
		fgl_api fgl_func_glVertexAttribI4uiv* fgl_glVertexAttribI4uiv;
#		define glVertexAttribI4uiv fgl_glVertexAttribI4uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4bv)(GLuint index, const GLbyte *v);
		fgl_api fgl_func_glVertexAttribI4bv* fgl_glVertexAttribI4bv;
#		define glVertexAttribI4bv fgl_glVertexAttribI4bv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4sv)(GLuint index, const GLshort *v);
		fgl_api fgl_func_glVertexAttribI4sv* fgl_glVertexAttribI4sv;
#		define glVertexAttribI4sv fgl_glVertexAttribI4sv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4ubv)(GLuint index, const GLubyte *v);
		fgl_api fgl_func_glVertexAttribI4ubv* fgl_glVertexAttribI4ubv;
#		define glVertexAttribI4ubv fgl_glVertexAttribI4ubv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribI4usv)(GLuint index, const GLushort *v);
		fgl_api fgl_func_glVertexAttribI4usv* fgl_glVertexAttribI4usv;
#		define glVertexAttribI4usv fgl_glVertexAttribI4usv
		typedef void (FGL_APIENTRY fgl_func_glGetUniformuiv)(GLuint program, GLint location, GLuint *params);
		fgl_api fgl_func_glGetUniformuiv* fgl_glGetUniformuiv;
#		define glGetUniformuiv fgl_glGetUniformuiv
		typedef void (FGL_APIENTRY fgl_func_glBindFragDataLocation)(GLuint program, GLuint color, const GLchar *name);
		fgl_api fgl_func_glBindFragDataLocation* fgl_glBindFragDataLocation;
#		define glBindFragDataLocation fgl_glBindFragDataLocation
		typedef GLint (FGL_APIENTRY fgl_func_glGetFragDataLocation)(GLuint program, const GLchar *name);
		fgl_api fgl_func_glGetFragDataLocation* fgl_glGetFragDataLocation;
#		define glGetFragDataLocation fgl_glGetFragDataLocation
		typedef void (FGL_APIENTRY fgl_func_glUniform1ui)(GLint location, GLuint v0);
		fgl_api fgl_func_glUniform1ui* fgl_glUniform1ui;
#		define glUniform1ui fgl_glUniform1ui
		typedef void (FGL_APIENTRY fgl_func_glUniform2ui)(GLint location, GLuint v0, GLuint v1);
		fgl_api fgl_func_glUniform2ui* fgl_glUniform2ui;
#		define glUniform2ui fgl_glUniform2ui
		typedef void (FGL_APIENTRY fgl_func_glUniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
		fgl_api fgl_func_glUniform3ui* fgl_glUniform3ui;
#		define glUniform3ui fgl_glUniform3ui
		typedef void (FGL_APIENTRY fgl_func_glUniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
		fgl_api fgl_func_glUniform4ui* fgl_glUniform4ui;
#		define glUniform4ui fgl_glUniform4ui
		typedef void (FGL_APIENTRY fgl_func_glUniform1uiv)(GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glUniform1uiv* fgl_glUniform1uiv;
#		define glUniform1uiv fgl_glUniform1uiv
		typedef void (FGL_APIENTRY fgl_func_glUniform2uiv)(GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glUniform2uiv* fgl_glUniform2uiv;
#		define glUniform2uiv fgl_glUniform2uiv
		typedef void (FGL_APIENTRY fgl_func_glUniform3uiv)(GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glUniform3uiv* fgl_glUniform3uiv;
#		define glUniform3uiv fgl_glUniform3uiv
		typedef void (FGL_APIENTRY fgl_func_glUniform4uiv)(GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glUniform4uiv* fgl_glUniform4uiv;
#		define glUniform4uiv fgl_glUniform4uiv
		typedef void (FGL_APIENTRY fgl_func_glTexParameterIiv)(GLenum target, GLenum pname, const GLint *params);
		fgl_api fgl_func_glTexParameterIiv* fgl_glTexParameterIiv;
#		define glTexParameterIiv fgl_glTexParameterIiv
		typedef void (FGL_APIENTRY fgl_func_glTexParameterIuiv)(GLenum target, GLenum pname, const GLuint *params);
		fgl_api fgl_func_glTexParameterIuiv* fgl_glTexParameterIuiv;
#		define glTexParameterIuiv fgl_glTexParameterIuiv
		typedef void (FGL_APIENTRY fgl_func_glGetTexParameterIiv)(GLenum target, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTexParameterIiv* fgl_glGetTexParameterIiv;
#		define glGetTexParameterIiv fgl_glGetTexParameterIiv
		typedef void (FGL_APIENTRY fgl_func_glGetTexParameterIuiv)(GLenum target, GLenum pname, GLuint *params);
		fgl_api fgl_func_glGetTexParameterIuiv* fgl_glGetTexParameterIuiv;
#		define glGetTexParameterIuiv fgl_glGetTexParameterIuiv
		typedef void (FGL_APIENTRY fgl_func_glClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint *value);
		fgl_api fgl_func_glClearBufferiv* fgl_glClearBufferiv;
#		define glClearBufferiv fgl_glClearBufferiv
		typedef void (FGL_APIENTRY fgl_func_glClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint *value);
		fgl_api fgl_func_glClearBufferuiv* fgl_glClearBufferuiv;
#		define glClearBufferuiv fgl_glClearBufferuiv
		typedef void (FGL_APIENTRY fgl_func_glClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
		fgl_api fgl_func_glClearBufferfv* fgl_glClearBufferfv;
#		define glClearBufferfv fgl_glClearBufferfv
		typedef void (FGL_APIENTRY fgl_func_glClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
		fgl_api fgl_func_glClearBufferfi* fgl_glClearBufferfi;
#		define glClearBufferfi fgl_glClearBufferfi
		typedef const GLubyte * (FGL_APIENTRY fgl_func_glGetStringi)(GLenum name, GLuint index);
		fgl_api fgl_func_glGetStringi* fgl_glGetStringi;
#		define glGetStringi fgl_glGetStringi
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsRenderbuffer)(GLuint renderbuffer);
		fgl_api fgl_func_glIsRenderbuffer* fgl_glIsRenderbuffer;
#		define glIsRenderbuffer fgl_glIsRenderbuffer
		typedef void (FGL_APIENTRY fgl_func_glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
		fgl_api fgl_func_glBindRenderbuffer* fgl_glBindRenderbuffer;
#		define glBindRenderbuffer fgl_glBindRenderbuffer
		typedef void (FGL_APIENTRY fgl_func_glDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers);
		fgl_api fgl_func_glDeleteRenderbuffers* fgl_glDeleteRenderbuffers;
#		define glDeleteRenderbuffers fgl_glDeleteRenderbuffers
		typedef void (FGL_APIENTRY fgl_func_glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers);
		fgl_api fgl_func_glGenRenderbuffers* fgl_glGenRenderbuffers;
#		define glGenRenderbuffers fgl_glGenRenderbuffers
		typedef void (FGL_APIENTRY fgl_func_glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
		fgl_api fgl_func_glRenderbufferStorage* fgl_glRenderbufferStorage;
#		define glRenderbufferStorage fgl_glRenderbufferStorage
		typedef void (FGL_APIENTRY fgl_func_glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetRenderbufferParameteriv* fgl_glGetRenderbufferParameteriv;
#		define glGetRenderbufferParameteriv fgl_glGetRenderbufferParameteriv
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsFramebuffer)(GLuint framebuffer);
		fgl_api fgl_func_glIsFramebuffer* fgl_glIsFramebuffer;
#		define glIsFramebuffer fgl_glIsFramebuffer
		typedef void (FGL_APIENTRY fgl_func_glBindFramebuffer)(GLenum target, GLuint framebuffer);
		fgl_api fgl_func_glBindFramebuffer* fgl_glBindFramebuffer;
#		define glBindFramebuffer fgl_glBindFramebuffer
		typedef void (FGL_APIENTRY fgl_func_glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);
		fgl_api fgl_func_glDeleteFramebuffers* fgl_glDeleteFramebuffers;
#		define glDeleteFramebuffers fgl_glDeleteFramebuffers
		typedef void (FGL_APIENTRY fgl_func_glGenFramebuffers)(GLsizei n, GLuint *framebuffers);
		fgl_api fgl_func_glGenFramebuffers* fgl_glGenFramebuffers;
#		define glGenFramebuffers fgl_glGenFramebuffers
		typedef GLenum (FGL_APIENTRY fgl_func_glCheckFramebufferStatus)(GLenum target);
		fgl_api fgl_func_glCheckFramebufferStatus* fgl_glCheckFramebufferStatus;
#		define glCheckFramebufferStatus fgl_glCheckFramebufferStatus
		typedef void (FGL_APIENTRY fgl_func_glFramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		fgl_api fgl_func_glFramebufferTexture1D* fgl_glFramebufferTexture1D;
#		define glFramebufferTexture1D fgl_glFramebufferTexture1D
		typedef void (FGL_APIENTRY fgl_func_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		fgl_api fgl_func_glFramebufferTexture2D* fgl_glFramebufferTexture2D;
#		define glFramebufferTexture2D fgl_glFramebufferTexture2D
		typedef void (FGL_APIENTRY fgl_func_glFramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
		fgl_api fgl_func_glFramebufferTexture3D* fgl_glFramebufferTexture3D;
#		define glFramebufferTexture3D fgl_glFramebufferTexture3D
		typedef void (FGL_APIENTRY fgl_func_glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		fgl_api fgl_func_glFramebufferRenderbuffer* fgl_glFramebufferRenderbuffer;
#		define glFramebufferRenderbuffer fgl_glFramebufferRenderbuffer
		typedef void (FGL_APIENTRY fgl_func_glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetFramebufferAttachmentParameteriv* fgl_glGetFramebufferAttachmentParameteriv;
#		define glGetFramebufferAttachmentParameteriv fgl_glGetFramebufferAttachmentParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGenerateMipmap)(GLenum target);
		fgl_api fgl_func_glGenerateMipmap* fgl_glGenerateMipmap;
#		define glGenerateMipmap fgl_glGenerateMipmap
		typedef void (FGL_APIENTRY fgl_func_glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
		fgl_api fgl_func_glBlitFramebuffer* fgl_glBlitFramebuffer;
#		define glBlitFramebuffer fgl_glBlitFramebuffer
		typedef void (FGL_APIENTRY fgl_func_glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
		fgl_api fgl_func_glRenderbufferStorageMultisample* fgl_glRenderbufferStorageMultisample;
#		define glRenderbufferStorageMultisample fgl_glRenderbufferStorageMultisample
		typedef void (FGL_APIENTRY fgl_func_glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
		fgl_api fgl_func_glFramebufferTextureLayer* fgl_glFramebufferTextureLayer;
#		define glFramebufferTextureLayer fgl_glFramebufferTextureLayer
		typedef void * (FGL_APIENTRY fgl_func_glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
		fgl_api fgl_func_glMapBufferRange* fgl_glMapBufferRange;
#		define glMapBufferRange fgl_glMapBufferRange
		typedef void (FGL_APIENTRY fgl_func_glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
		fgl_api fgl_func_glFlushMappedBufferRange* fgl_glFlushMappedBufferRange;
#		define glFlushMappedBufferRange fgl_glFlushMappedBufferRange
		typedef void (FGL_APIENTRY fgl_func_glBindVertexArray)(GLuint array);
		fgl_api fgl_func_glBindVertexArray* fgl_glBindVertexArray;
#		define glBindVertexArray fgl_glBindVertexArray
		typedef void (FGL_APIENTRY fgl_func_glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
		fgl_api fgl_func_glDeleteVertexArrays* fgl_glDeleteVertexArrays;
#		define glDeleteVertexArrays fgl_glDeleteVertexArrays
		typedef void (FGL_APIENTRY fgl_func_glGenVertexArrays)(GLsizei n, GLuint *arrays);
		fgl_api fgl_func_glGenVertexArrays* fgl_glGenVertexArrays;
#		define glGenVertexArrays fgl_glGenVertexArrays
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsVertexArray)(GLuint array);
		fgl_api fgl_func_glIsVertexArray* fgl_glIsVertexArray;
#		define glIsVertexArray fgl_glIsVertexArray
#	endif // GL_VERSION_3_0

#	ifndef GL_VERSION_3_1
#		define GL_VERSION_3_1 1
		fgl_api bool isGL_VERSION_3_1;

#		define GL_SAMPLER_2D_RECT 0x8B63
#		define GL_SAMPLER_2D_RECT_SHADOW 0x8B64
#		define GL_SAMPLER_BUFFER 0x8DC2
#		define GL_INT_SAMPLER_2D_RECT 0x8DCD
#		define GL_INT_SAMPLER_BUFFER 0x8DD0
#		define GL_UNSIGNED_INT_SAMPLER_2D_RECT 0x8DD5
#		define GL_UNSIGNED_INT_SAMPLER_BUFFER 0x8DD8
#		define GL_TEXTURE_BUFFER 0x8C2A
#		define GL_MAX_TEXTURE_BUFFER_SIZE 0x8C2B
#		define GL_TEXTURE_BINDING_BUFFER 0x8C2C
#		define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#		define GL_TEXTURE_RECTANGLE 0x84F5
#		define GL_TEXTURE_BINDING_RECTANGLE 0x84F6
#		define GL_PROXY_TEXTURE_RECTANGLE 0x84F7
#		define GL_MAX_RECTANGLE_TEXTURE_SIZE 0x84F8
#		define GL_R8_SNORM 0x8F94
#		define GL_RG8_SNORM 0x8F95
#		define GL_RGB8_SNORM 0x8F96
#		define GL_RGBA8_SNORM 0x8F97
#		define GL_R16_SNORM 0x8F98
#		define GL_RG16_SNORM 0x8F99
#		define GL_RGB16_SNORM 0x8F9A
#		define GL_RGBA16_SNORM 0x8F9B
#		define GL_SIGNED_NORMALIZED 0x8F9C
#		define GL_PRIMITIVE_RESTART 0x8F9D
#		define GL_PRIMITIVE_RESTART_INDEX 0x8F9E
#		define GL_COPY_READ_BUFFER 0x8F36
#		define GL_COPY_WRITE_BUFFER 0x8F37
#		define GL_UNIFORM_BUFFER 0x8A11
#		define GL_UNIFORM_BUFFER_BINDING 0x8A28
#		define GL_UNIFORM_BUFFER_START 0x8A29
#		define GL_UNIFORM_BUFFER_SIZE 0x8A2A
#		define GL_MAX_VERTEX_UNIFORM_BLOCKS 0x8A2B
#		define GL_MAX_GEOMETRY_UNIFORM_BLOCKS 0x8A2C
#		define GL_MAX_FRAGMENT_UNIFORM_BLOCKS 0x8A2D
#		define GL_MAX_COMBINED_UNIFORM_BLOCKS 0x8A2E
#		define GL_MAX_UNIFORM_BUFFER_BINDINGS 0x8A2F
#		define GL_MAX_UNIFORM_BLOCK_SIZE 0x8A30
#		define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#		define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#		define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#		define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#		define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#		define GL_ACTIVE_UNIFORM_BLOCKS 0x8A36
#		define GL_UNIFORM_TYPE 0x8A37
#		define GL_UNIFORM_SIZE 0x8A38
#		define GL_UNIFORM_NAME_LENGTH 0x8A39
#		define GL_UNIFORM_BLOCK_INDEX 0x8A3A
#		define GL_UNIFORM_OFFSET 0x8A3B
#		define GL_UNIFORM_ARRAY_STRIDE 0x8A3C
#		define GL_UNIFORM_MATRIX_STRIDE 0x8A3D
#		define GL_UNIFORM_IS_ROW_MAJOR 0x8A3E
#		define GL_UNIFORM_BLOCK_BINDING 0x8A3F
#		define GL_UNIFORM_BLOCK_DATA_SIZE 0x8A40
#		define GL_UNIFORM_BLOCK_NAME_LENGTH 0x8A41
#		define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS 0x8A42
#		define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#		define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#		define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER 0x8A45
#		define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#		define GL_INVALID_INDEX 0xFFFFFFFFu

		typedef void (FGL_APIENTRY fgl_func_glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
		fgl_api fgl_func_glDrawArraysInstanced* fgl_glDrawArraysInstanced;
#		define glDrawArraysInstanced fgl_glDrawArraysInstanced
		typedef void (FGL_APIENTRY fgl_func_glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
		fgl_api fgl_func_glDrawElementsInstanced* fgl_glDrawElementsInstanced;
#		define glDrawElementsInstanced fgl_glDrawElementsInstanced
		typedef void (FGL_APIENTRY fgl_func_glTexBuffer)(GLenum target, GLenum internalformat, GLuint buffer);
		fgl_api fgl_func_glTexBuffer* fgl_glTexBuffer;
#		define glTexBuffer fgl_glTexBuffer
		typedef void (FGL_APIENTRY fgl_func_glPrimitiveRestartIndex)(GLuint index);
		fgl_api fgl_func_glPrimitiveRestartIndex* fgl_glPrimitiveRestartIndex;
#		define glPrimitiveRestartIndex fgl_glPrimitiveRestartIndex
		typedef void (FGL_APIENTRY fgl_func_glCopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
		fgl_api fgl_func_glCopyBufferSubData* fgl_glCopyBufferSubData;
#		define glCopyBufferSubData fgl_glCopyBufferSubData
		typedef void (FGL_APIENTRY fgl_func_glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
		fgl_api fgl_func_glGetUniformIndices* fgl_glGetUniformIndices;
#		define glGetUniformIndices fgl_glGetUniformIndices
		typedef void (FGL_APIENTRY fgl_func_glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetActiveUniformsiv* fgl_glGetActiveUniformsiv;
#		define glGetActiveUniformsiv fgl_glGetActiveUniformsiv
		typedef void (FGL_APIENTRY fgl_func_glGetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
		fgl_api fgl_func_glGetActiveUniformName* fgl_glGetActiveUniformName;
#		define glGetActiveUniformName fgl_glGetActiveUniformName
		typedef GLuint (FGL_APIENTRY fgl_func_glGetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName);
		fgl_api fgl_func_glGetUniformBlockIndex* fgl_glGetUniformBlockIndex;
#		define glGetUniformBlockIndex fgl_glGetUniformBlockIndex
		typedef void (FGL_APIENTRY fgl_func_glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetActiveUniformBlockiv* fgl_glGetActiveUniformBlockiv;
#		define glGetActiveUniformBlockiv fgl_glGetActiveUniformBlockiv
		typedef void (FGL_APIENTRY fgl_func_glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
		fgl_api fgl_func_glGetActiveUniformBlockName* fgl_glGetActiveUniformBlockName;
#		define glGetActiveUniformBlockName fgl_glGetActiveUniformBlockName
		typedef void (FGL_APIENTRY fgl_func_glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
		fgl_api fgl_func_glUniformBlockBinding* fgl_glUniformBlockBinding;
#		define glUniformBlockBinding fgl_glUniformBlockBinding
#	endif // GL_VERSION_3_1

#	ifndef GL_VERSION_3_2
#		define GL_VERSION_3_2 1
		fgl_api bool isGL_VERSION_3_2;

		typedef struct __GLsync *GLsync;
		typedef uint64_t GLuint64;
		typedef int64_t GLint64;

#		define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#		define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#		define GL_LINES_ADJACENCY 0x000A
#		define GL_LINE_STRIP_ADJACENCY 0x000B
#		define GL_TRIANGLES_ADJACENCY 0x000C
#		define GL_TRIANGLE_STRIP_ADJACENCY 0x000D
#		define GL_PROGRAM_POINT_SIZE 0x8642
#		define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#		define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#		define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#		define GL_GEOMETRY_SHADER 0x8DD9
#		define GL_GEOMETRY_VERTICES_OUT 0x8916
#		define GL_GEOMETRY_INPUT_TYPE 0x8917
#		define GL_GEOMETRY_OUTPUT_TYPE 0x8918
#		define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#		define GL_MAX_GEOMETRY_OUTPUT_VERTICES 0x8DE0
#		define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#		define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#		define GL_MAX_GEOMETRY_INPUT_COMPONENTS 0x9123
#		define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#		define GL_MAX_FRAGMENT_INPUT_COMPONENTS 0x9125
#		define GL_CONTEXT_PROFILE_MASK 0x9126
#		define GL_DEPTH_CLAMP 0x864F
#		define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#		define GL_FIRST_VERTEX_CONVENTION 0x8E4D
#		define GL_LAST_VERTEX_CONVENTION 0x8E4E
#		define GL_PROVOKING_VERTEX 0x8E4F
#		define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#		define GL_MAX_SERVER_WAIT_TIMEOUT 0x9111
#		define GL_OBJECT_TYPE 0x9112
#		define GL_SYNC_CONDITION 0x9113
#		define GL_SYNC_STATUS 0x9114
#		define GL_SYNC_FLAGS 0x9115
#		define GL_SYNC_FENCE 0x9116
#		define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#		define GL_UNSIGNALED 0x9118
#		define GL_SIGNALED 0x9119
#		define GL_ALREADY_SIGNALED 0x911A
#		define GL_TIMEOUT_EXPIRED 0x911B
#		define GL_CONDITION_SATISFIED 0x911C
#		define GL_WAIT_FAILED 0x911D
#		define GL_TIMEOUT_IGNORED 0xFFFFFFFFFFFFFFFFull
#		define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#		define GL_SAMPLE_POSITION 0x8E50
#		define GL_SAMPLE_MASK 0x8E51
#		define GL_SAMPLE_MASK_VALUE 0x8E52
#		define GL_MAX_SAMPLE_MASK_WORDS 0x8E59
#		define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#		define GL_PROXY_TEXTURE_2D_MULTISAMPLE 0x9101
#		define GL_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9102
#		define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9103
#		define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#		define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
#		define GL_TEXTURE_SAMPLES 0x9106
#		define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#		define GL_SAMPLER_2D_MULTISAMPLE 0x9108
#		define GL_INT_SAMPLER_2D_MULTISAMPLE 0x9109
#		define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#		define GL_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910B
#		define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#		define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#		define GL_MAX_COLOR_TEXTURE_SAMPLES 0x910E
#		define GL_MAX_DEPTH_TEXTURE_SAMPLES 0x910F
#		define GL_MAX_INTEGER_SAMPLES 0x9110

		typedef void (FGL_APIENTRY fgl_func_glDrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
		fgl_api fgl_func_glDrawElementsBaseVertex* fgl_glDrawElementsBaseVertex;
#		define glDrawElementsBaseVertex fgl_glDrawElementsBaseVertex
		typedef void (FGL_APIENTRY fgl_func_glDrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
		fgl_api fgl_func_glDrawRangeElementsBaseVertex* fgl_glDrawRangeElementsBaseVertex;
#		define glDrawRangeElementsBaseVertex fgl_glDrawRangeElementsBaseVertex
		typedef void (FGL_APIENTRY fgl_func_glDrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
		fgl_api fgl_func_glDrawElementsInstancedBaseVertex* fgl_glDrawElementsInstancedBaseVertex;
#		define glDrawElementsInstancedBaseVertex fgl_glDrawElementsInstancedBaseVertex
		typedef void (FGL_APIENTRY fgl_func_glMultiDrawElementsBaseVertex)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
		fgl_api fgl_func_glMultiDrawElementsBaseVertex* fgl_glMultiDrawElementsBaseVertex;
#		define glMultiDrawElementsBaseVertex fgl_glMultiDrawElementsBaseVertex
		typedef void (FGL_APIENTRY fgl_func_glProvokingVertex)(GLenum mode);
		fgl_api fgl_func_glProvokingVertex* fgl_glProvokingVertex;
#		define glProvokingVertex fgl_glProvokingVertex
		typedef GLsync (FGL_APIENTRY fgl_func_glFenceSync)(GLenum condition, GLbitfield flags);
		fgl_api fgl_func_glFenceSync* fgl_glFenceSync;
#		define glFenceSync fgl_glFenceSync
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsSync)(GLsync sync);
		fgl_api fgl_func_glIsSync* fgl_glIsSync;
#		define glIsSync fgl_glIsSync
		typedef void (FGL_APIENTRY fgl_func_glDeleteSync)(GLsync sync);
		fgl_api fgl_func_glDeleteSync* fgl_glDeleteSync;
#		define glDeleteSync fgl_glDeleteSync
		typedef GLenum (FGL_APIENTRY fgl_func_glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
		fgl_api fgl_func_glClientWaitSync* fgl_glClientWaitSync;
#		define glClientWaitSync fgl_glClientWaitSync
		typedef void (FGL_APIENTRY fgl_func_glWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
		fgl_api fgl_func_glWaitSync* fgl_glWaitSync;
#		define glWaitSync fgl_glWaitSync
		typedef void (FGL_APIENTRY fgl_func_glGetInteger64v)(GLenum pname, GLint64 *data);
		fgl_api fgl_func_glGetInteger64v* fgl_glGetInteger64v;
#		define glGetInteger64v fgl_glGetInteger64v
		typedef void (FGL_APIENTRY fgl_func_glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
		fgl_api fgl_func_glGetSynciv* fgl_glGetSynciv;
#		define glGetSynciv fgl_glGetSynciv
		typedef void (FGL_APIENTRY fgl_func_glGetInteger64i_v)(GLenum target, GLuint index, GLint64 *data);
		fgl_api fgl_func_glGetInteger64i_v* fgl_glGetInteger64i_v;
#		define glGetInteger64i_v fgl_glGetInteger64i_v
		typedef void (FGL_APIENTRY fgl_func_glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params);
		fgl_api fgl_func_glGetBufferParameteri64v* fgl_glGetBufferParameteri64v;
#		define glGetBufferParameteri64v fgl_glGetBufferParameteri64v
		typedef void (FGL_APIENTRY fgl_func_glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);
		fgl_api fgl_func_glFramebufferTexture* fgl_glFramebufferTexture;
#		define glFramebufferTexture fgl_glFramebufferTexture
		typedef void (FGL_APIENTRY fgl_func_glTexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		fgl_api fgl_func_glTexImage2DMultisample* fgl_glTexImage2DMultisample;
#		define glTexImage2DMultisample fgl_glTexImage2DMultisample
		typedef void (FGL_APIENTRY fgl_func_glTexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		fgl_api fgl_func_glTexImage3DMultisample* fgl_glTexImage3DMultisample;
#		define glTexImage3DMultisample fgl_glTexImage3DMultisample
		typedef void (FGL_APIENTRY fgl_func_glGetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val);
		fgl_api fgl_func_glGetMultisamplefv* fgl_glGetMultisamplefv;
#		define glGetMultisamplefv fgl_glGetMultisamplefv
		typedef void (FGL_APIENTRY fgl_func_glSampleMaski)(GLuint maskNumber, GLbitfield mask);
		fgl_api fgl_func_glSampleMaski* fgl_glSampleMaski;
#		define glSampleMaski fgl_glSampleMaski
#	endif // GL_VERSION_3_2

#	ifndef GL_VERSION_3_3
#		define GL_VERSION_3_3 1
		fgl_api bool isGL_VERSION_3_3;

#		define GL_VERTEX_ATTRIB_ARRAY_DIVISOR 0x88FE
#		define GL_SRC1_COLOR 0x88F9
#		define GL_ONE_MINUS_SRC1_COLOR 0x88FA
#		define GL_ONE_MINUS_SRC1_ALPHA 0x88FB
#		define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS 0x88FC
#		define GL_ANY_SAMPLES_PASSED 0x8C2F
#		define GL_SAMPLER_BINDING 0x8919
#		define GL_RGB10_A2UI 0x906F
#		define GL_TEXTURE_SWIZZLE_R 0x8E42
#		define GL_TEXTURE_SWIZZLE_G 0x8E43
#		define GL_TEXTURE_SWIZZLE_B 0x8E44
#		define GL_TEXTURE_SWIZZLE_A 0x8E45
#		define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#		define GL_TIME_ELAPSED 0x88BF
#		define GL_TIMESTAMP 0x8E28
#		define GL_INT_2_10_10_10_REV 0x8D9F

		typedef void (FGL_APIENTRY fgl_func_glBindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
		fgl_api fgl_func_glBindFragDataLocationIndexed* fgl_glBindFragDataLocationIndexed;
#		define glBindFragDataLocationIndexed fgl_glBindFragDataLocationIndexed
		typedef GLint (FGL_APIENTRY fgl_func_glGetFragDataIndex)(GLuint program, const GLchar *name);
		fgl_api fgl_func_glGetFragDataIndex* fgl_glGetFragDataIndex;
#		define glGetFragDataIndex fgl_glGetFragDataIndex
		typedef void (FGL_APIENTRY fgl_func_glGenSamplers)(GLsizei count, GLuint *samplers);
		fgl_api fgl_func_glGenSamplers* fgl_glGenSamplers;
#		define glGenSamplers fgl_glGenSamplers
		typedef void (FGL_APIENTRY fgl_func_glDeleteSamplers)(GLsizei count, const GLuint *samplers);
		fgl_api fgl_func_glDeleteSamplers* fgl_glDeleteSamplers;
#		define glDeleteSamplers fgl_glDeleteSamplers
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsSampler)(GLuint sampler);
		fgl_api fgl_func_glIsSampler* fgl_glIsSampler;
#		define glIsSampler fgl_glIsSampler
		typedef void (FGL_APIENTRY fgl_func_glBindSampler)(GLuint unit, GLuint sampler);
		fgl_api fgl_func_glBindSampler* fgl_glBindSampler;
#		define glBindSampler fgl_glBindSampler
		typedef void (FGL_APIENTRY fgl_func_glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
		fgl_api fgl_func_glSamplerParameteri* fgl_glSamplerParameteri;
#		define glSamplerParameteri fgl_glSamplerParameteri
		typedef void (FGL_APIENTRY fgl_func_glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *param);
		fgl_api fgl_func_glSamplerParameteriv* fgl_glSamplerParameteriv;
#		define glSamplerParameteriv fgl_glSamplerParameteriv
		typedef void (FGL_APIENTRY fgl_func_glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
		fgl_api fgl_func_glSamplerParameterf* fgl_glSamplerParameterf;
#		define glSamplerParameterf fgl_glSamplerParameterf
		typedef void (FGL_APIENTRY fgl_func_glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *param);
		fgl_api fgl_func_glSamplerParameterfv* fgl_glSamplerParameterfv;
#		define glSamplerParameterfv fgl_glSamplerParameterfv
		typedef void (FGL_APIENTRY fgl_func_glSamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint *param);
		fgl_api fgl_func_glSamplerParameterIiv* fgl_glSamplerParameterIiv;
#		define glSamplerParameterIiv fgl_glSamplerParameterIiv
		typedef void (FGL_APIENTRY fgl_func_glSamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint *param);
		fgl_api fgl_func_glSamplerParameterIuiv* fgl_glSamplerParameterIuiv;
#		define glSamplerParameterIuiv fgl_glSamplerParameterIuiv
		typedef void (FGL_APIENTRY fgl_func_glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetSamplerParameteriv* fgl_glGetSamplerParameteriv;
#		define glGetSamplerParameteriv fgl_glGetSamplerParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetSamplerParameterIiv* fgl_glGetSamplerParameterIiv;
#		define glGetSamplerParameterIiv fgl_glGetSamplerParameterIiv
		typedef void (FGL_APIENTRY fgl_func_glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetSamplerParameterfv* fgl_glGetSamplerParameterfv;
#		define glGetSamplerParameterfv fgl_glGetSamplerParameterfv
		typedef void (FGL_APIENTRY fgl_func_glGetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint *params);
		fgl_api fgl_func_glGetSamplerParameterIuiv* fgl_glGetSamplerParameterIuiv;
#		define glGetSamplerParameterIuiv fgl_glGetSamplerParameterIuiv
		typedef void (FGL_APIENTRY fgl_func_glQueryCounter)(GLuint id, GLenum target);
		fgl_api fgl_func_glQueryCounter* fgl_glQueryCounter;
#		define glQueryCounter fgl_glQueryCounter
		typedef void (FGL_APIENTRY fgl_func_glGetQueryObjecti64v)(GLuint id, GLenum pname, GLint64 *params);
		fgl_api fgl_func_glGetQueryObjecti64v* fgl_glGetQueryObjecti64v;
#		define glGetQueryObjecti64v fgl_glGetQueryObjecti64v
		typedef void (FGL_APIENTRY fgl_func_glGetQueryObjectui64v)(GLuint id, GLenum pname, GLuint64 *params);
		fgl_api fgl_func_glGetQueryObjectui64v* fgl_glGetQueryObjectui64v;
#		define glGetQueryObjectui64v fgl_glGetQueryObjectui64v
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribDivisor)(GLuint index, GLuint divisor);
		fgl_api fgl_func_glVertexAttribDivisor* fgl_glVertexAttribDivisor;
#		define glVertexAttribDivisor fgl_glVertexAttribDivisor
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		fgl_api fgl_func_glVertexAttribP1ui* fgl_glVertexAttribP1ui;
#		define glVertexAttribP1ui fgl_glVertexAttribP1ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
		fgl_api fgl_func_glVertexAttribP1uiv* fgl_glVertexAttribP1uiv;
#		define glVertexAttribP1uiv fgl_glVertexAttribP1uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		fgl_api fgl_func_glVertexAttribP2ui* fgl_glVertexAttribP2ui;
#		define glVertexAttribP2ui fgl_glVertexAttribP2ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
		fgl_api fgl_func_glVertexAttribP2uiv* fgl_glVertexAttribP2uiv;
#		define glVertexAttribP2uiv fgl_glVertexAttribP2uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		fgl_api fgl_func_glVertexAttribP3ui* fgl_glVertexAttribP3ui;
#		define glVertexAttribP3ui fgl_glVertexAttribP3ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
		fgl_api fgl_func_glVertexAttribP3uiv* fgl_glVertexAttribP3uiv;
#		define glVertexAttribP3uiv fgl_glVertexAttribP3uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		fgl_api fgl_func_glVertexAttribP4ui* fgl_glVertexAttribP4ui;
#		define glVertexAttribP4ui fgl_glVertexAttribP4ui
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
		fgl_api fgl_func_glVertexAttribP4uiv* fgl_glVertexAttribP4uiv;
#		define glVertexAttribP4uiv fgl_glVertexAttribP4uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexP2ui)(GLenum type, GLuint value);
		fgl_api fgl_func_glVertexP2ui* fgl_glVertexP2ui;
#		define glVertexP2ui fgl_glVertexP2ui
		typedef void (FGL_APIENTRY fgl_func_glVertexP2uiv)(GLenum type, const GLuint *value);
		fgl_api fgl_func_glVertexP2uiv* fgl_glVertexP2uiv;
#		define glVertexP2uiv fgl_glVertexP2uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexP3ui)(GLenum type, GLuint value);
		fgl_api fgl_func_glVertexP3ui* fgl_glVertexP3ui;
#		define glVertexP3ui fgl_glVertexP3ui
		typedef void (FGL_APIENTRY fgl_func_glVertexP3uiv)(GLenum type, const GLuint *value);
		fgl_api fgl_func_glVertexP3uiv* fgl_glVertexP3uiv;
#		define glVertexP3uiv fgl_glVertexP3uiv
		typedef void (FGL_APIENTRY fgl_func_glVertexP4ui)(GLenum type, GLuint value);
		fgl_api fgl_func_glVertexP4ui* fgl_glVertexP4ui;
#		define glVertexP4ui fgl_glVertexP4ui
		typedef void (FGL_APIENTRY fgl_func_glVertexP4uiv)(GLenum type, const GLuint *value);
		fgl_api fgl_func_glVertexP4uiv* fgl_glVertexP4uiv;
#		define glVertexP4uiv fgl_glVertexP4uiv
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP1ui)(GLenum type, GLuint coords);
		fgl_api fgl_func_glTexCoordP1ui* fgl_glTexCoordP1ui;
#		define glTexCoordP1ui fgl_glTexCoordP1ui
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP1uiv)(GLenum type, const GLuint *coords);
		fgl_api fgl_func_glTexCoordP1uiv* fgl_glTexCoordP1uiv;
#		define glTexCoordP1uiv fgl_glTexCoordP1uiv
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP2ui)(GLenum type, GLuint coords);
		fgl_api fgl_func_glTexCoordP2ui* fgl_glTexCoordP2ui;
#		define glTexCoordP2ui fgl_glTexCoordP2ui
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP2uiv)(GLenum type, const GLuint *coords);
		fgl_api fgl_func_glTexCoordP2uiv* fgl_glTexCoordP2uiv;
#		define glTexCoordP2uiv fgl_glTexCoordP2uiv
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP3ui)(GLenum type, GLuint coords);
		fgl_api fgl_func_glTexCoordP3ui* fgl_glTexCoordP3ui;
#		define glTexCoordP3ui fgl_glTexCoordP3ui
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP3uiv)(GLenum type, const GLuint *coords);
		fgl_api fgl_func_glTexCoordP3uiv* fgl_glTexCoordP3uiv;
#		define glTexCoordP3uiv fgl_glTexCoordP3uiv
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP4ui)(GLenum type, GLuint coords);
		fgl_api fgl_func_glTexCoordP4ui* fgl_glTexCoordP4ui;
#		define glTexCoordP4ui fgl_glTexCoordP4ui
		typedef void (FGL_APIENTRY fgl_func_glTexCoordP4uiv)(GLenum type, const GLuint *coords);
		fgl_api fgl_func_glTexCoordP4uiv* fgl_glTexCoordP4uiv;
#		define glTexCoordP4uiv fgl_glTexCoordP4uiv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP1ui)(GLenum texture, GLenum type, GLuint coords);
		fgl_api fgl_func_glMultiTexCoordP1ui* fgl_glMultiTexCoordP1ui;
#		define glMultiTexCoordP1ui fgl_glMultiTexCoordP1ui
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP1uiv)(GLenum texture, GLenum type, const GLuint *coords);
		fgl_api fgl_func_glMultiTexCoordP1uiv* fgl_glMultiTexCoordP1uiv;
#		define glMultiTexCoordP1uiv fgl_glMultiTexCoordP1uiv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP2ui)(GLenum texture, GLenum type, GLuint coords);
		fgl_api fgl_func_glMultiTexCoordP2ui* fgl_glMultiTexCoordP2ui;
#		define glMultiTexCoordP2ui fgl_glMultiTexCoordP2ui
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP2uiv)(GLenum texture, GLenum type, const GLuint *coords);
		fgl_api fgl_func_glMultiTexCoordP2uiv* fgl_glMultiTexCoordP2uiv;
#		define glMultiTexCoordP2uiv fgl_glMultiTexCoordP2uiv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP3ui)(GLenum texture, GLenum type, GLuint coords);
		fgl_api fgl_func_glMultiTexCoordP3ui* fgl_glMultiTexCoordP3ui;
#		define glMultiTexCoordP3ui fgl_glMultiTexCoordP3ui
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP3uiv)(GLenum texture, GLenum type, const GLuint *coords);
		fgl_api fgl_func_glMultiTexCoordP3uiv* fgl_glMultiTexCoordP3uiv;
#		define glMultiTexCoordP3uiv fgl_glMultiTexCoordP3uiv
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP4ui)(GLenum texture, GLenum type, GLuint coords);
		fgl_api fgl_func_glMultiTexCoordP4ui* fgl_glMultiTexCoordP4ui;
#		define glMultiTexCoordP4ui fgl_glMultiTexCoordP4ui
		typedef void (FGL_APIENTRY fgl_func_glMultiTexCoordP4uiv)(GLenum texture, GLenum type, const GLuint *coords);
		fgl_api fgl_func_glMultiTexCoordP4uiv* fgl_glMultiTexCoordP4uiv;
#		define glMultiTexCoordP4uiv fgl_glMultiTexCoordP4uiv
		typedef void (FGL_APIENTRY fgl_func_glNormalP3ui)(GLenum type, GLuint coords);
		fgl_api fgl_func_glNormalP3ui* fgl_glNormalP3ui;
#		define glNormalP3ui fgl_glNormalP3ui
		typedef void (FGL_APIENTRY fgl_func_glNormalP3uiv)(GLenum type, const GLuint *coords);
		fgl_api fgl_func_glNormalP3uiv* fgl_glNormalP3uiv;
#		define glNormalP3uiv fgl_glNormalP3uiv
		typedef void (FGL_APIENTRY fgl_func_glColorP3ui)(GLenum type, GLuint color);
		fgl_api fgl_func_glColorP3ui* fgl_glColorP3ui;
#		define glColorP3ui fgl_glColorP3ui
		typedef void (FGL_APIENTRY fgl_func_glColorP3uiv)(GLenum type, const GLuint *color);
		fgl_api fgl_func_glColorP3uiv* fgl_glColorP3uiv;
#		define glColorP3uiv fgl_glColorP3uiv
		typedef void (FGL_APIENTRY fgl_func_glColorP4ui)(GLenum type, GLuint color);
		fgl_api fgl_func_glColorP4ui* fgl_glColorP4ui;
#		define glColorP4ui fgl_glColorP4ui
		typedef void (FGL_APIENTRY fgl_func_glColorP4uiv)(GLenum type, const GLuint *color);
		fgl_api fgl_func_glColorP4uiv* fgl_glColorP4uiv;
#		define glColorP4uiv fgl_glColorP4uiv
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColorP3ui)(GLenum type, GLuint color);
		fgl_api fgl_func_glSecondaryColorP3ui* fgl_glSecondaryColorP3ui;
#		define glSecondaryColorP3ui fgl_glSecondaryColorP3ui
		typedef void (FGL_APIENTRY fgl_func_glSecondaryColorP3uiv)(GLenum type, const GLuint *color);
		fgl_api fgl_func_glSecondaryColorP3uiv* fgl_glSecondaryColorP3uiv;
#		define glSecondaryColorP3uiv fgl_glSecondaryColorP3uiv
#	endif // GL_VERSION_3_3

#	ifndef GL_VERSION_4_0
#		define GL_VERSION_4_0 1
		fgl_api bool isGL_VERSION_4_0;

#		define GL_SAMPLE_SHADING 0x8C36
#		define GL_MIN_SAMPLE_SHADING_VALUE 0x8C37
#		define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#		define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#		define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#		define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#		define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY 0x900B
#		define GL_SAMPLER_CUBE_MAP_ARRAY 0x900C
#		define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW 0x900D
#		define GL_INT_SAMPLER_CUBE_MAP_ARRAY 0x900E
#		define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F
#		define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#		define GL_DRAW_INDIRECT_BUFFER_BINDING 0x8F43
#		define GL_GEOMETRY_SHADER_INVOCATIONS 0x887F
#		define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A
#		define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET 0x8E5B
#		define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET 0x8E5C
#		define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS 0x8E5D
#		define GL_MAX_VERTEX_STREAMS 0x8E71
#		define GL_DOUBLE_VEC2 0x8FFC
#		define GL_DOUBLE_VEC3 0x8FFD
#		define GL_DOUBLE_VEC4 0x8FFE
#		define GL_DOUBLE_MAT2 0x8F46
#		define GL_DOUBLE_MAT3 0x8F47
#		define GL_DOUBLE_MAT4 0x8F48
#		define GL_DOUBLE_MAT2x3 0x8F49
#		define GL_DOUBLE_MAT2x4 0x8F4A
#		define GL_DOUBLE_MAT3x2 0x8F4B
#		define GL_DOUBLE_MAT3x4 0x8F4C
#		define GL_DOUBLE_MAT4x2 0x8F4D
#		define GL_DOUBLE_MAT4x3 0x8F4E
#		define GL_ACTIVE_SUBROUTINES 0x8DE5
#		define GL_ACTIVE_SUBROUTINE_UNIFORMS 0x8DE6
#		define GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS 0x8E47
#		define GL_ACTIVE_SUBROUTINE_MAX_LENGTH 0x8E48
#		define GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH 0x8E49
#		define GL_MAX_SUBROUTINES 0x8DE7
#		define GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS 0x8DE8
#		define GL_NUM_COMPATIBLE_SUBROUTINES 0x8E4A
#		define GL_COMPATIBLE_SUBROUTINES 0x8E4B
#		define GL_PATCHES 0x000E
#		define GL_PATCH_VERTICES 0x8E72
#		define GL_PATCH_DEFAULT_INNER_LEVEL 0x8E73
#		define GL_PATCH_DEFAULT_OUTER_LEVEL 0x8E74
#		define GL_TESS_CONTROL_OUTPUT_VERTICES 0x8E75
#		define GL_TESS_GEN_MODE 0x8E76
#		define GL_TESS_GEN_SPACING 0x8E77
#		define GL_TESS_GEN_VERTEX_ORDER 0x8E78
#		define GL_TESS_GEN_POINT_MODE 0x8E79
#		define GL_ISOLINES 0x8E7A
#		define GL_FRACTIONAL_ODD 0x8E7B
#		define GL_FRACTIONAL_EVEN 0x8E7C
#		define GL_MAX_PATCH_VERTICES 0x8E7D
#		define GL_MAX_TESS_GEN_LEVEL 0x8E7E
#		define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E7F
#		define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E80
#		define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS 0x8E81
#		define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS 0x8E82
#		define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS 0x8E83
#		define GL_MAX_TESS_PATCH_COMPONENTS 0x8E84
#		define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS 0x8E85
#		define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS 0x8E86
#		define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS 0x8E89
#		define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS 0x8E8A
#		define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS 0x886C
#		define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS 0x886D
#		define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E1E
#		define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E1F
#		define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER 0x84F0
#		define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER 0x84F1
#		define GL_TESS_EVALUATION_SHADER 0x8E87
#		define GL_TESS_CONTROL_SHADER 0x8E88
#		define GL_TRANSFORM_FEEDBACK 0x8E22
#		define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED 0x8E23
#		define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE 0x8E24
#		define GL_TRANSFORM_FEEDBACK_BINDING 0x8E25
#		define GL_MAX_TRANSFORM_FEEDBACK_BUFFERS 0x8E70

		typedef void (FGL_APIENTRY fgl_func_glMinSampleShading)(GLfloat value);
		fgl_api fgl_func_glMinSampleShading* fgl_glMinSampleShading;
#		define glMinSampleShading fgl_glMinSampleShading
		typedef void (FGL_APIENTRY fgl_func_glBlendEquationi)(GLuint buf, GLenum mode);
		fgl_api fgl_func_glBlendEquationi* fgl_glBlendEquationi;
#		define glBlendEquationi fgl_glBlendEquationi
		typedef void (FGL_APIENTRY fgl_func_glBlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
		fgl_api fgl_func_glBlendEquationSeparatei* fgl_glBlendEquationSeparatei;
#		define glBlendEquationSeparatei fgl_glBlendEquationSeparatei
		typedef void (FGL_APIENTRY fgl_func_glBlendFunci)(GLuint buf, GLenum src, GLenum dst);
		fgl_api fgl_func_glBlendFunci* fgl_glBlendFunci;
#		define glBlendFunci fgl_glBlendFunci
		typedef void (FGL_APIENTRY fgl_func_glBlendFuncSeparatei)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
		fgl_api fgl_func_glBlendFuncSeparatei* fgl_glBlendFuncSeparatei;
#		define glBlendFuncSeparatei fgl_glBlendFuncSeparatei
		typedef void (FGL_APIENTRY fgl_func_glDrawArraysIndirect)(GLenum mode, const void *indirect);
		fgl_api fgl_func_glDrawArraysIndirect* fgl_glDrawArraysIndirect;
#		define glDrawArraysIndirect fgl_glDrawArraysIndirect
		typedef void (FGL_APIENTRY fgl_func_glDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect);
		fgl_api fgl_func_glDrawElementsIndirect* fgl_glDrawElementsIndirect;
#		define glDrawElementsIndirect fgl_glDrawElementsIndirect
		typedef void (FGL_APIENTRY fgl_func_glUniform1d)(GLint location, GLdouble x);
		fgl_api fgl_func_glUniform1d* fgl_glUniform1d;
#		define glUniform1d fgl_glUniform1d
		typedef void (FGL_APIENTRY fgl_func_glUniform2d)(GLint location, GLdouble x, GLdouble y);
		fgl_api fgl_func_glUniform2d* fgl_glUniform2d;
#		define glUniform2d fgl_glUniform2d
		typedef void (FGL_APIENTRY fgl_func_glUniform3d)(GLint location, GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glUniform3d* fgl_glUniform3d;
#		define glUniform3d fgl_glUniform3d
		typedef void (FGL_APIENTRY fgl_func_glUniform4d)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		fgl_api fgl_func_glUniform4d* fgl_glUniform4d;
#		define glUniform4d fgl_glUniform4d
		typedef void (FGL_APIENTRY fgl_func_glUniform1dv)(GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glUniform1dv* fgl_glUniform1dv;
#		define glUniform1dv fgl_glUniform1dv
		typedef void (FGL_APIENTRY fgl_func_glUniform2dv)(GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glUniform2dv* fgl_glUniform2dv;
#		define glUniform2dv fgl_glUniform2dv
		typedef void (FGL_APIENTRY fgl_func_glUniform3dv)(GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glUniform3dv* fgl_glUniform3dv;
#		define glUniform3dv fgl_glUniform3dv
		typedef void (FGL_APIENTRY fgl_func_glUniform4dv)(GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glUniform4dv* fgl_glUniform4dv;
#		define glUniform4dv fgl_glUniform4dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix2dv* fgl_glUniformMatrix2dv;
#		define glUniformMatrix2dv fgl_glUniformMatrix2dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix3dv* fgl_glUniformMatrix3dv;
#		define glUniformMatrix3dv fgl_glUniformMatrix3dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix4dv* fgl_glUniformMatrix4dv;
#		define glUniformMatrix4dv fgl_glUniformMatrix4dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix2x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix2x3dv* fgl_glUniformMatrix2x3dv;
#		define glUniformMatrix2x3dv fgl_glUniformMatrix2x3dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix2x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix2x4dv* fgl_glUniformMatrix2x4dv;
#		define glUniformMatrix2x4dv fgl_glUniformMatrix2x4dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix3x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix3x2dv* fgl_glUniformMatrix3x2dv;
#		define glUniformMatrix3x2dv fgl_glUniformMatrix3x2dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix3x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix3x4dv* fgl_glUniformMatrix3x4dv;
#		define glUniformMatrix3x4dv fgl_glUniformMatrix3x4dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix4x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix4x2dv* fgl_glUniformMatrix4x2dv;
#		define glUniformMatrix4x2dv fgl_glUniformMatrix4x2dv
		typedef void (FGL_APIENTRY fgl_func_glUniformMatrix4x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glUniformMatrix4x3dv* fgl_glUniformMatrix4x3dv;
#		define glUniformMatrix4x3dv fgl_glUniformMatrix4x3dv
		typedef void (FGL_APIENTRY fgl_func_glGetUniformdv)(GLuint program, GLint location, GLdouble *params);
		fgl_api fgl_func_glGetUniformdv* fgl_glGetUniformdv;
#		define glGetUniformdv fgl_glGetUniformdv
		typedef GLint (FGL_APIENTRY fgl_func_glGetSubroutineUniformLocation)(GLuint program, GLenum shadertype, const GLchar *name);
		fgl_api fgl_func_glGetSubroutineUniformLocation* fgl_glGetSubroutineUniformLocation;
#		define glGetSubroutineUniformLocation fgl_glGetSubroutineUniformLocation
		typedef GLuint (FGL_APIENTRY fgl_func_glGetSubroutineIndex)(GLuint program, GLenum shadertype, const GLchar *name);
		fgl_api fgl_func_glGetSubroutineIndex* fgl_glGetSubroutineIndex;
#		define glGetSubroutineIndex fgl_glGetSubroutineIndex
		typedef void (FGL_APIENTRY fgl_func_glGetActiveSubroutineUniformiv)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
		fgl_api fgl_func_glGetActiveSubroutineUniformiv* fgl_glGetActiveSubroutineUniformiv;
#		define glGetActiveSubroutineUniformiv fgl_glGetActiveSubroutineUniformiv
		typedef void (FGL_APIENTRY fgl_func_glGetActiveSubroutineUniformName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
		fgl_api fgl_func_glGetActiveSubroutineUniformName* fgl_glGetActiveSubroutineUniformName;
#		define glGetActiveSubroutineUniformName fgl_glGetActiveSubroutineUniformName
		typedef void (FGL_APIENTRY fgl_func_glGetActiveSubroutineName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
		fgl_api fgl_func_glGetActiveSubroutineName* fgl_glGetActiveSubroutineName;
#		define glGetActiveSubroutineName fgl_glGetActiveSubroutineName
		typedef void (FGL_APIENTRY fgl_func_glUniformSubroutinesuiv)(GLenum shadertype, GLsizei count, const GLuint *indices);
		fgl_api fgl_func_glUniformSubroutinesuiv* fgl_glUniformSubroutinesuiv;
#		define glUniformSubroutinesuiv fgl_glUniformSubroutinesuiv
		typedef void (FGL_APIENTRY fgl_func_glGetUniformSubroutineuiv)(GLenum shadertype, GLint location, GLuint *params);
		fgl_api fgl_func_glGetUniformSubroutineuiv* fgl_glGetUniformSubroutineuiv;
#		define glGetUniformSubroutineuiv fgl_glGetUniformSubroutineuiv
		typedef void (FGL_APIENTRY fgl_func_glGetProgramStageiv)(GLuint program, GLenum shadertype, GLenum pname, GLint *values);
		fgl_api fgl_func_glGetProgramStageiv* fgl_glGetProgramStageiv;
#		define glGetProgramStageiv fgl_glGetProgramStageiv
		typedef void (FGL_APIENTRY fgl_func_glPatchParameteri)(GLenum pname, GLint value);
		fgl_api fgl_func_glPatchParameteri* fgl_glPatchParameteri;
#		define glPatchParameteri fgl_glPatchParameteri
		typedef void (FGL_APIENTRY fgl_func_glPatchParameterfv)(GLenum pname, const GLfloat *values);
		fgl_api fgl_func_glPatchParameterfv* fgl_glPatchParameterfv;
#		define glPatchParameterfv fgl_glPatchParameterfv
		typedef void (FGL_APIENTRY fgl_func_glBindTransformFeedback)(GLenum target, GLuint id);
		fgl_api fgl_func_glBindTransformFeedback* fgl_glBindTransformFeedback;
#		define glBindTransformFeedback fgl_glBindTransformFeedback
		typedef void (FGL_APIENTRY fgl_func_glDeleteTransformFeedbacks)(GLsizei n, const GLuint *ids);
		fgl_api fgl_func_glDeleteTransformFeedbacks* fgl_glDeleteTransformFeedbacks;
#		define glDeleteTransformFeedbacks fgl_glDeleteTransformFeedbacks
		typedef void (FGL_APIENTRY fgl_func_glGenTransformFeedbacks)(GLsizei n, GLuint *ids);
		fgl_api fgl_func_glGenTransformFeedbacks* fgl_glGenTransformFeedbacks;
#		define glGenTransformFeedbacks fgl_glGenTransformFeedbacks
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsTransformFeedback)(GLuint id);
		fgl_api fgl_func_glIsTransformFeedback* fgl_glIsTransformFeedback;
#		define glIsTransformFeedback fgl_glIsTransformFeedback
		typedef void (FGL_APIENTRY fgl_func_glPauseTransformFeedback)(void);
		fgl_api fgl_func_glPauseTransformFeedback* fgl_glPauseTransformFeedback;
#		define glPauseTransformFeedback fgl_glPauseTransformFeedback
		typedef void (FGL_APIENTRY fgl_func_glResumeTransformFeedback)(void);
		fgl_api fgl_func_glResumeTransformFeedback* fgl_glResumeTransformFeedback;
#		define glResumeTransformFeedback fgl_glResumeTransformFeedback
		typedef void (FGL_APIENTRY fgl_func_glDrawTransformFeedback)(GLenum mode, GLuint id);
		fgl_api fgl_func_glDrawTransformFeedback* fgl_glDrawTransformFeedback;
#		define glDrawTransformFeedback fgl_glDrawTransformFeedback
		typedef void (FGL_APIENTRY fgl_func_glDrawTransformFeedbackStream)(GLenum mode, GLuint id, GLuint stream);
		fgl_api fgl_func_glDrawTransformFeedbackStream* fgl_glDrawTransformFeedbackStream;
#		define glDrawTransformFeedbackStream fgl_glDrawTransformFeedbackStream
		typedef void (FGL_APIENTRY fgl_func_glBeginQueryIndexed)(GLenum target, GLuint index, GLuint id);
		fgl_api fgl_func_glBeginQueryIndexed* fgl_glBeginQueryIndexed;
#		define glBeginQueryIndexed fgl_glBeginQueryIndexed
		typedef void (FGL_APIENTRY fgl_func_glEndQueryIndexed)(GLenum target, GLuint index);
		fgl_api fgl_func_glEndQueryIndexed* fgl_glEndQueryIndexed;
#		define glEndQueryIndexed fgl_glEndQueryIndexed
		typedef void (FGL_APIENTRY fgl_func_glGetQueryIndexediv)(GLenum target, GLuint index, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetQueryIndexediv* fgl_glGetQueryIndexediv;
#		define glGetQueryIndexediv fgl_glGetQueryIndexediv
#	endif // GL_VERSION_4_0

#	ifndef GL_VERSION_4_1
#		define GL_VERSION_4_1 1
		fgl_api bool isGL_VERSION_4_1;

#		define GL_FIXED 0x140C
#		define GL_IMPLEMENTATION_COLOR_READ_TYPE 0x8B9A
#		define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#		define GL_LOW_FLOAT 0x8DF0
#		define GL_MEDIUM_FLOAT 0x8DF1
#		define GL_HIGH_FLOAT 0x8DF2
#		define GL_LOW_INT 0x8DF3
#		define GL_MEDIUM_INT 0x8DF4
#		define GL_HIGH_INT 0x8DF5
#		define GL_SHADER_COMPILER 0x8DFA
#		define GL_SHADER_BINARY_FORMATS 0x8DF8
#		define GL_NUM_SHADER_BINARY_FORMATS 0x8DF9
#		define GL_MAX_VERTEX_UNIFORM_VECTORS 0x8DFB
#		define GL_MAX_VARYING_VECTORS 0x8DFC
#		define GL_MAX_FRAGMENT_UNIFORM_VECTORS 0x8DFD
#		define GL_RGB565 0x8D62
#		define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#		define GL_PROGRAM_BINARY_LENGTH 0x8741
#		define GL_NUM_PROGRAM_BINARY_FORMATS 0x87FE
#		define GL_PROGRAM_BINARY_FORMATS 0x87FF
#		define GL_VERTEX_SHADER_BIT 0x00000001
#		define GL_FRAGMENT_SHADER_BIT 0x00000002
#		define GL_GEOMETRY_SHADER_BIT 0x00000004
#		define GL_TESS_CONTROL_SHADER_BIT 0x00000008
#		define GL_TESS_EVALUATION_SHADER_BIT 0x00000010
#		define GL_ALL_SHADER_BITS 0xFFFFFFFF
#		define GL_PROGRAM_SEPARABLE 0x8258
#		define GL_ACTIVE_PROGRAM 0x8259
#		define GL_PROGRAM_PIPELINE_BINDING 0x825A
#		define GL_MAX_VIEWPORTS 0x825B
#		define GL_VIEWPORT_SUBPIXEL_BITS 0x825C
#		define GL_VIEWPORT_BOUNDS_RANGE 0x825D
#		define GL_LAYER_PROVOKING_VERTEX 0x825E
#		define GL_VIEWPORT_INDEX_PROVOKING_VERTEX 0x825F
#		define GL_UNDEFINED_VERTEX 0x8260

		typedef void (FGL_APIENTRY fgl_func_glReleaseShaderCompiler)(void);
		fgl_api fgl_func_glReleaseShaderCompiler* fgl_glReleaseShaderCompiler;
#		define glReleaseShaderCompiler fgl_glReleaseShaderCompiler
		typedef void (FGL_APIENTRY fgl_func_glShaderBinary)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
		fgl_api fgl_func_glShaderBinary* fgl_glShaderBinary;
#		define glShaderBinary fgl_glShaderBinary
		typedef void (FGL_APIENTRY fgl_func_glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
		fgl_api fgl_func_glGetShaderPrecisionFormat* fgl_glGetShaderPrecisionFormat;
#		define glGetShaderPrecisionFormat fgl_glGetShaderPrecisionFormat
		typedef void (FGL_APIENTRY fgl_func_glDepthRangef)(GLfloat n, GLfloat f);
		fgl_api fgl_func_glDepthRangef* fgl_glDepthRangef;
#		define glDepthRangef fgl_glDepthRangef
		typedef void (FGL_APIENTRY fgl_func_glClearDepthf)(GLfloat d);
		fgl_api fgl_func_glClearDepthf* fgl_glClearDepthf;
#		define glClearDepthf fgl_glClearDepthf
		typedef void (FGL_APIENTRY fgl_func_glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
		fgl_api fgl_func_glGetProgramBinary* fgl_glGetProgramBinary;
#		define glGetProgramBinary fgl_glGetProgramBinary
		typedef void (FGL_APIENTRY fgl_func_glProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
		fgl_api fgl_func_glProgramBinary* fgl_glProgramBinary;
#		define glProgramBinary fgl_glProgramBinary
		typedef void (FGL_APIENTRY fgl_func_glProgramParameteri)(GLuint program, GLenum pname, GLint value);
		fgl_api fgl_func_glProgramParameteri* fgl_glProgramParameteri;
#		define glProgramParameteri fgl_glProgramParameteri
		typedef void (FGL_APIENTRY fgl_func_glUseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
		fgl_api fgl_func_glUseProgramStages* fgl_glUseProgramStages;
#		define glUseProgramStages fgl_glUseProgramStages
		typedef void (FGL_APIENTRY fgl_func_glActiveShaderProgram)(GLuint pipeline, GLuint program);
		fgl_api fgl_func_glActiveShaderProgram* fgl_glActiveShaderProgram;
#		define glActiveShaderProgram fgl_glActiveShaderProgram
		typedef GLuint (FGL_APIENTRY fgl_func_glCreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const*strings);
		fgl_api fgl_func_glCreateShaderProgramv* fgl_glCreateShaderProgramv;
#		define glCreateShaderProgramv fgl_glCreateShaderProgramv
		typedef void (FGL_APIENTRY fgl_func_glBindProgramPipeline)(GLuint pipeline);
		fgl_api fgl_func_glBindProgramPipeline* fgl_glBindProgramPipeline;
#		define glBindProgramPipeline fgl_glBindProgramPipeline
		typedef void (FGL_APIENTRY fgl_func_glDeleteProgramPipelines)(GLsizei n, const GLuint *pipelines);
		fgl_api fgl_func_glDeleteProgramPipelines* fgl_glDeleteProgramPipelines;
#		define glDeleteProgramPipelines fgl_glDeleteProgramPipelines
		typedef void (FGL_APIENTRY fgl_func_glGenProgramPipelines)(GLsizei n, GLuint *pipelines);
		fgl_api fgl_func_glGenProgramPipelines* fgl_glGenProgramPipelines;
#		define glGenProgramPipelines fgl_glGenProgramPipelines
		typedef GLboolean (FGL_APIENTRY fgl_func_glIsProgramPipeline)(GLuint pipeline);
		fgl_api fgl_func_glIsProgramPipeline* fgl_glIsProgramPipeline;
#		define glIsProgramPipeline fgl_glIsProgramPipeline
		typedef void (FGL_APIENTRY fgl_func_glGetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetProgramPipelineiv* fgl_glGetProgramPipelineiv;
#		define glGetProgramPipelineiv fgl_glGetProgramPipelineiv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1i)(GLuint program, GLint location, GLint v0);
		fgl_api fgl_func_glProgramUniform1i* fgl_glProgramUniform1i;
#		define glProgramUniform1i fgl_glProgramUniform1i
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glProgramUniform1iv* fgl_glProgramUniform1iv;
#		define glProgramUniform1iv fgl_glProgramUniform1iv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
		fgl_api fgl_func_glProgramUniform1f* fgl_glProgramUniform1f;
#		define glProgramUniform1f fgl_glProgramUniform1f
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glProgramUniform1fv* fgl_glProgramUniform1fv;
#		define glProgramUniform1fv fgl_glProgramUniform1fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1d)(GLuint program, GLint location, GLdouble v0);
		fgl_api fgl_func_glProgramUniform1d* fgl_glProgramUniform1d;
#		define glProgramUniform1d fgl_glProgramUniform1d
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glProgramUniform1dv* fgl_glProgramUniform1dv;
#		define glProgramUniform1dv fgl_glProgramUniform1dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
		fgl_api fgl_func_glProgramUniform1ui* fgl_glProgramUniform1ui;
#		define glProgramUniform1ui fgl_glProgramUniform1ui
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glProgramUniform1uiv* fgl_glProgramUniform1uiv;
#		define glProgramUniform1uiv fgl_glProgramUniform1uiv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
		fgl_api fgl_func_glProgramUniform2i* fgl_glProgramUniform2i;
#		define glProgramUniform2i fgl_glProgramUniform2i
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glProgramUniform2iv* fgl_glProgramUniform2iv;
#		define glProgramUniform2iv fgl_glProgramUniform2iv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
		fgl_api fgl_func_glProgramUniform2f* fgl_glProgramUniform2f;
#		define glProgramUniform2f fgl_glProgramUniform2f
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glProgramUniform2fv* fgl_glProgramUniform2fv;
#		define glProgramUniform2fv fgl_glProgramUniform2fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2d)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
		fgl_api fgl_func_glProgramUniform2d* fgl_glProgramUniform2d;
#		define glProgramUniform2d fgl_glProgramUniform2d
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glProgramUniform2dv* fgl_glProgramUniform2dv;
#		define glProgramUniform2dv fgl_glProgramUniform2dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
		fgl_api fgl_func_glProgramUniform2ui* fgl_glProgramUniform2ui;
#		define glProgramUniform2ui fgl_glProgramUniform2ui
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glProgramUniform2uiv* fgl_glProgramUniform2uiv;
#		define glProgramUniform2uiv fgl_glProgramUniform2uiv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
		fgl_api fgl_func_glProgramUniform3i* fgl_glProgramUniform3i;
#		define glProgramUniform3i fgl_glProgramUniform3i
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glProgramUniform3iv* fgl_glProgramUniform3iv;
#		define glProgramUniform3iv fgl_glProgramUniform3iv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
		fgl_api fgl_func_glProgramUniform3f* fgl_glProgramUniform3f;
#		define glProgramUniform3f fgl_glProgramUniform3f
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glProgramUniform3fv* fgl_glProgramUniform3fv;
#		define glProgramUniform3fv fgl_glProgramUniform3fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
		fgl_api fgl_func_glProgramUniform3d* fgl_glProgramUniform3d;
#		define glProgramUniform3d fgl_glProgramUniform3d
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glProgramUniform3dv* fgl_glProgramUniform3dv;
#		define glProgramUniform3dv fgl_glProgramUniform3dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
		fgl_api fgl_func_glProgramUniform3ui* fgl_glProgramUniform3ui;
#		define glProgramUniform3ui fgl_glProgramUniform3ui
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glProgramUniform3uiv* fgl_glProgramUniform3uiv;
#		define glProgramUniform3uiv fgl_glProgramUniform3uiv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
		fgl_api fgl_func_glProgramUniform4i* fgl_glProgramUniform4i;
#		define glProgramUniform4i fgl_glProgramUniform4i
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
		fgl_api fgl_func_glProgramUniform4iv* fgl_glProgramUniform4iv;
#		define glProgramUniform4iv fgl_glProgramUniform4iv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
		fgl_api fgl_func_glProgramUniform4f* fgl_glProgramUniform4f;
#		define glProgramUniform4f fgl_glProgramUniform4f
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
		fgl_api fgl_func_glProgramUniform4fv* fgl_glProgramUniform4fv;
#		define glProgramUniform4fv fgl_glProgramUniform4fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
		fgl_api fgl_func_glProgramUniform4d* fgl_glProgramUniform4d;
#		define glProgramUniform4d fgl_glProgramUniform4d
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
		fgl_api fgl_func_glProgramUniform4dv* fgl_glProgramUniform4dv;
#		define glProgramUniform4dv fgl_glProgramUniform4dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
		fgl_api fgl_func_glProgramUniform4ui* fgl_glProgramUniform4ui;
#		define glProgramUniform4ui fgl_glProgramUniform4ui
		typedef void (FGL_APIENTRY fgl_func_glProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
		fgl_api fgl_func_glProgramUniform4uiv* fgl_glProgramUniform4uiv;
#		define glProgramUniform4uiv fgl_glProgramUniform4uiv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix2fv* fgl_glProgramUniformMatrix2fv;
#		define glProgramUniformMatrix2fv fgl_glProgramUniformMatrix2fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix3fv* fgl_glProgramUniformMatrix3fv;
#		define glProgramUniformMatrix3fv fgl_glProgramUniformMatrix3fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix4fv* fgl_glProgramUniformMatrix4fv;
#		define glProgramUniformMatrix4fv fgl_glProgramUniformMatrix4fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix2dv* fgl_glProgramUniformMatrix2dv;
#		define glProgramUniformMatrix2dv fgl_glProgramUniformMatrix2dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix3dv* fgl_glProgramUniformMatrix3dv;
#		define glProgramUniformMatrix3dv fgl_glProgramUniformMatrix3dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix4dv* fgl_glProgramUniformMatrix4dv;
#		define glProgramUniformMatrix4dv fgl_glProgramUniformMatrix4dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix2x3fv* fgl_glProgramUniformMatrix2x3fv;
#		define glProgramUniformMatrix2x3fv fgl_glProgramUniformMatrix2x3fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix3x2fv* fgl_glProgramUniformMatrix3x2fv;
#		define glProgramUniformMatrix3x2fv fgl_glProgramUniformMatrix3x2fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix2x4fv* fgl_glProgramUniformMatrix2x4fv;
#		define glProgramUniformMatrix2x4fv fgl_glProgramUniformMatrix2x4fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix4x2fv* fgl_glProgramUniformMatrix4x2fv;
#		define glProgramUniformMatrix4x2fv fgl_glProgramUniformMatrix4x2fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix3x4fv* fgl_glProgramUniformMatrix3x4fv;
#		define glProgramUniformMatrix3x4fv fgl_glProgramUniformMatrix3x4fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		fgl_api fgl_func_glProgramUniformMatrix4x3fv* fgl_glProgramUniformMatrix4x3fv;
#		define glProgramUniformMatrix4x3fv fgl_glProgramUniformMatrix4x3fv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix2x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix2x3dv* fgl_glProgramUniformMatrix2x3dv;
#		define glProgramUniformMatrix2x3dv fgl_glProgramUniformMatrix2x3dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix3x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix3x2dv* fgl_glProgramUniformMatrix3x2dv;
#		define glProgramUniformMatrix3x2dv fgl_glProgramUniformMatrix3x2dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix2x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix2x4dv* fgl_glProgramUniformMatrix2x4dv;
#		define glProgramUniformMatrix2x4dv fgl_glProgramUniformMatrix2x4dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix4x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix4x2dv* fgl_glProgramUniformMatrix4x2dv;
#		define glProgramUniformMatrix4x2dv fgl_glProgramUniformMatrix4x2dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix3x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix3x4dv* fgl_glProgramUniformMatrix3x4dv;
#		define glProgramUniformMatrix3x4dv fgl_glProgramUniformMatrix3x4dv
		typedef void (FGL_APIENTRY fgl_func_glProgramUniformMatrix4x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		fgl_api fgl_func_glProgramUniformMatrix4x3dv* fgl_glProgramUniformMatrix4x3dv;
#		define glProgramUniformMatrix4x3dv fgl_glProgramUniformMatrix4x3dv
		typedef void (FGL_APIENTRY fgl_func_glValidateProgramPipeline)(GLuint pipeline);
		fgl_api fgl_func_glValidateProgramPipeline* fgl_glValidateProgramPipeline;
#		define glValidateProgramPipeline fgl_glValidateProgramPipeline
		typedef void (FGL_APIENTRY fgl_func_glGetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
		fgl_api fgl_func_glGetProgramPipelineInfoLog* fgl_glGetProgramPipelineInfoLog;
#		define glGetProgramPipelineInfoLog fgl_glGetProgramPipelineInfoLog
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL1d)(GLuint index, GLdouble x);
		fgl_api fgl_func_glVertexAttribL1d* fgl_glVertexAttribL1d;
#		define glVertexAttribL1d fgl_glVertexAttribL1d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL2d)(GLuint index, GLdouble x, GLdouble y);
		fgl_api fgl_func_glVertexAttribL2d* fgl_glVertexAttribL2d;
#		define glVertexAttribL2d fgl_glVertexAttribL2d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
		fgl_api fgl_func_glVertexAttribL3d* fgl_glVertexAttribL3d;
#		define glVertexAttribL3d fgl_glVertexAttribL3d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		fgl_api fgl_func_glVertexAttribL4d* fgl_glVertexAttribL4d;
#		define glVertexAttribL4d fgl_glVertexAttribL4d
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL1dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttribL1dv* fgl_glVertexAttribL1dv;
#		define glVertexAttribL1dv fgl_glVertexAttribL1dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL2dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttribL2dv* fgl_glVertexAttribL2dv;
#		define glVertexAttribL2dv fgl_glVertexAttribL2dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL3dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttribL3dv* fgl_glVertexAttribL3dv;
#		define glVertexAttribL3dv fgl_glVertexAttribL3dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribL4dv)(GLuint index, const GLdouble *v);
		fgl_api fgl_func_glVertexAttribL4dv* fgl_glVertexAttribL4dv;
#		define glVertexAttribL4dv fgl_glVertexAttribL4dv
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribLPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
		fgl_api fgl_func_glVertexAttribLPointer* fgl_glVertexAttribLPointer;
#		define glVertexAttribLPointer fgl_glVertexAttribLPointer
		typedef void (FGL_APIENTRY fgl_func_glGetVertexAttribLdv)(GLuint index, GLenum pname, GLdouble *params);
		fgl_api fgl_func_glGetVertexAttribLdv* fgl_glGetVertexAttribLdv;
#		define glGetVertexAttribLdv fgl_glGetVertexAttribLdv
		typedef void (FGL_APIENTRY fgl_func_glViewportArrayv)(GLuint first, GLsizei count, const GLfloat *v);
		fgl_api fgl_func_glViewportArrayv* fgl_glViewportArrayv;
#		define glViewportArrayv fgl_glViewportArrayv
		typedef void (FGL_APIENTRY fgl_func_glViewportIndexedf)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
		fgl_api fgl_func_glViewportIndexedf* fgl_glViewportIndexedf;
#		define glViewportIndexedf fgl_glViewportIndexedf
		typedef void (FGL_APIENTRY fgl_func_glViewportIndexedfv)(GLuint index, const GLfloat *v);
		fgl_api fgl_func_glViewportIndexedfv* fgl_glViewportIndexedfv;
#		define glViewportIndexedfv fgl_glViewportIndexedfv
		typedef void (FGL_APIENTRY fgl_func_glScissorArrayv)(GLuint first, GLsizei count, const GLint *v);
		fgl_api fgl_func_glScissorArrayv* fgl_glScissorArrayv;
#		define glScissorArrayv fgl_glScissorArrayv
		typedef void (FGL_APIENTRY fgl_func_glScissorIndexed)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
		fgl_api fgl_func_glScissorIndexed* fgl_glScissorIndexed;
#		define glScissorIndexed fgl_glScissorIndexed
		typedef void (FGL_APIENTRY fgl_func_glScissorIndexedv)(GLuint index, const GLint *v);
		fgl_api fgl_func_glScissorIndexedv* fgl_glScissorIndexedv;
#		define glScissorIndexedv fgl_glScissorIndexedv
		typedef void (FGL_APIENTRY fgl_func_glDepthRangeArrayv)(GLuint first, GLsizei count, const GLdouble *v);
		fgl_api fgl_func_glDepthRangeArrayv* fgl_glDepthRangeArrayv;
#		define glDepthRangeArrayv fgl_glDepthRangeArrayv
		typedef void (FGL_APIENTRY fgl_func_glDepthRangeIndexed)(GLuint index, GLdouble n, GLdouble f);
		fgl_api fgl_func_glDepthRangeIndexed* fgl_glDepthRangeIndexed;
#		define glDepthRangeIndexed fgl_glDepthRangeIndexed
		typedef void (FGL_APIENTRY fgl_func_glGetFloati_v)(GLenum target, GLuint index, GLfloat *data);
		fgl_api fgl_func_glGetFloati_v* fgl_glGetFloati_v;
#		define glGetFloati_v fgl_glGetFloati_v
		typedef void (FGL_APIENTRY fgl_func_glGetDoublei_v)(GLenum target, GLuint index, GLdouble *data);
		fgl_api fgl_func_glGetDoublei_v* fgl_glGetDoublei_v;
#		define glGetDoublei_v fgl_glGetDoublei_v
#	endif // GL_VERSION_4_1

#	ifndef GL_VERSION_4_2
#		define GL_VERSION_4_2 1
		fgl_api bool isGL_VERSION_4_2;

#		define GL_COPY_READ_BUFFER_BINDING 0x8F36
#		define GL_COPY_WRITE_BUFFER_BINDING 0x8F37
#		define GL_TRANSFORM_FEEDBACK_ACTIVE 0x8E24
#		define GL_TRANSFORM_FEEDBACK_PAUSED 0x8E23
#		define GL_UNPACK_COMPRESSED_BLOCK_WIDTH 0x9127
#		define GL_UNPACK_COMPRESSED_BLOCK_HEIGHT 0x9128
#		define GL_UNPACK_COMPRESSED_BLOCK_DEPTH 0x9129
#		define GL_UNPACK_COMPRESSED_BLOCK_SIZE 0x912A
#		define GL_PACK_COMPRESSED_BLOCK_WIDTH 0x912B
#		define GL_PACK_COMPRESSED_BLOCK_HEIGHT 0x912C
#		define GL_PACK_COMPRESSED_BLOCK_DEPTH 0x912D
#		define GL_PACK_COMPRESSED_BLOCK_SIZE 0x912E
#		define GL_NUM_SAMPLE_COUNTS 0x9380
#		define GL_MIN_MAP_BUFFER_ALIGNMENT 0x90BC
#		define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#		define GL_ATOMIC_COUNTER_BUFFER_BINDING 0x92C1
#		define GL_ATOMIC_COUNTER_BUFFER_START 0x92C2
#		define GL_ATOMIC_COUNTER_BUFFER_SIZE 0x92C3
#		define GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE 0x92C4
#		define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS 0x92C5
#		define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES 0x92C6
#		define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER 0x92C7
#		define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER 0x92C8
#		define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER 0x92C9
#		define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER 0x92CA
#		define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER 0x92CB
#		define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#		define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS 0x92CD
#		define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS 0x92CE
#		define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS 0x92CF
#		define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#		define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#		define GL_MAX_VERTEX_ATOMIC_COUNTERS 0x92D2
#		define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS 0x92D3
#		define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS 0x92D4
#		define GL_MAX_GEOMETRY_ATOMIC_COUNTERS 0x92D5
#		define GL_MAX_FRAGMENT_ATOMIC_COUNTERS 0x92D6
#		define GL_MAX_COMBINED_ATOMIC_COUNTERS 0x92D7
#		define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#		define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#		define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS 0x92D9
#		define GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX 0x92DA
#		define GL_UNSIGNED_INT_ATOMIC_COUNTER 0x92DB
#		define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#		define GL_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#		define GL_UNIFORM_BARRIER_BIT 0x00000004
#		define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#		define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#		define GL_COMMAND_BARRIER_BIT 0x00000040
#		define GL_PIXEL_BUFFER_BARRIER_BIT 0x00000080
#		define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#		define GL_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#		define GL_FRAMEBUFFER_BARRIER_BIT 0x00000400
#		define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#		define GL_ATOMIC_COUNTER_BARRIER_BIT 0x00001000
#		define GL_ALL_BARRIER_BITS 0xFFFFFFFF
#		define GL_MAX_IMAGE_UNITS 0x8F38
#		define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS 0x8F39
#		define GL_IMAGE_BINDING_NAME 0x8F3A
#		define GL_IMAGE_BINDING_LEVEL 0x8F3B
#		define GL_IMAGE_BINDING_LAYERED 0x8F3C
#		define GL_IMAGE_BINDING_LAYER 0x8F3D
#		define GL_IMAGE_BINDING_ACCESS 0x8F3E
#		define GL_IMAGE_1D 0x904C
#		define GL_IMAGE_2D 0x904D
#		define GL_IMAGE_3D 0x904E
#		define GL_IMAGE_2D_RECT 0x904F
#		define GL_IMAGE_CUBE 0x9050
#		define GL_IMAGE_BUFFER 0x9051
#		define GL_IMAGE_1D_ARRAY 0x9052
#		define GL_IMAGE_2D_ARRAY 0x9053
#		define GL_IMAGE_CUBE_MAP_ARRAY 0x9054
#		define GL_IMAGE_2D_MULTISAMPLE 0x9055
#		define GL_IMAGE_2D_MULTISAMPLE_ARRAY 0x9056
#		define GL_INT_IMAGE_1D 0x9057
#		define GL_INT_IMAGE_2D 0x9058
#		define GL_INT_IMAGE_3D 0x9059
#		define GL_INT_IMAGE_2D_RECT 0x905A
#		define GL_INT_IMAGE_CUBE 0x905B
#		define GL_INT_IMAGE_BUFFER 0x905C
#		define GL_INT_IMAGE_1D_ARRAY 0x905D
#		define GL_INT_IMAGE_2D_ARRAY 0x905E
#		define GL_INT_IMAGE_CUBE_MAP_ARRAY 0x905F
#		define GL_INT_IMAGE_2D_MULTISAMPLE 0x9060
#		define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x9061
#		define GL_UNSIGNED_INT_IMAGE_1D 0x9062
#		define GL_UNSIGNED_INT_IMAGE_2D 0x9063
#		define GL_UNSIGNED_INT_IMAGE_3D 0x9064
#		define GL_UNSIGNED_INT_IMAGE_2D_RECT 0x9065
#		define GL_UNSIGNED_INT_IMAGE_CUBE 0x9066
#		define GL_UNSIGNED_INT_IMAGE_BUFFER 0x9067
#		define GL_UNSIGNED_INT_IMAGE_1D_ARRAY 0x9068
#		define GL_UNSIGNED_INT_IMAGE_2D_ARRAY 0x9069
#		define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A
#		define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE 0x906B
#		define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x906C
#		define GL_MAX_IMAGE_SAMPLES 0x906D
#		define GL_IMAGE_BINDING_FORMAT 0x906E
#		define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
#		define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
#		define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
#		define GL_MAX_VERTEX_IMAGE_UNIFORMS 0x90CA
#		define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS 0x90CB
#		define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS 0x90CC
#		define GL_MAX_GEOMETRY_IMAGE_UNIFORMS 0x90CD
#		define GL_MAX_FRAGMENT_IMAGE_UNIFORMS 0x90CE
#		define GL_MAX_COMBINED_IMAGE_UNIFORMS 0x90CF
#		define GL_COMPRESSED_RGBA_BPTC_UNORM 0x8E8C
#		define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#		define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#		define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F
#		define GL_TEXTURE_IMMUTABLE_FORMAT 0x912F

		typedef void (FGL_APIENTRY fgl_func_glDrawArraysInstancedBaseInstance)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
		fgl_api fgl_func_glDrawArraysInstancedBaseInstance* fgl_glDrawArraysInstancedBaseInstance;
#		define glDrawArraysInstancedBaseInstance fgl_glDrawArraysInstancedBaseInstance
		typedef void (FGL_APIENTRY fgl_func_glDrawElementsInstancedBaseInstance)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
		fgl_api fgl_func_glDrawElementsInstancedBaseInstance* fgl_glDrawElementsInstancedBaseInstance;
#		define glDrawElementsInstancedBaseInstance fgl_glDrawElementsInstancedBaseInstance
		typedef void (FGL_APIENTRY fgl_func_glDrawElementsInstancedBaseVertexBaseInstance)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
		fgl_api fgl_func_glDrawElementsInstancedBaseVertexBaseInstance* fgl_glDrawElementsInstancedBaseVertexBaseInstance;
#		define glDrawElementsInstancedBaseVertexBaseInstance fgl_glDrawElementsInstancedBaseVertexBaseInstance
		typedef void (FGL_APIENTRY fgl_func_glGetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
		fgl_api fgl_func_glGetInternalformativ* fgl_glGetInternalformativ;
#		define glGetInternalformativ fgl_glGetInternalformativ
		typedef void (FGL_APIENTRY fgl_func_glGetActiveAtomicCounterBufferiv)(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetActiveAtomicCounterBufferiv* fgl_glGetActiveAtomicCounterBufferiv;
#		define glGetActiveAtomicCounterBufferiv fgl_glGetActiveAtomicCounterBufferiv
		typedef void (FGL_APIENTRY fgl_func_glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
		fgl_api fgl_func_glBindImageTexture* fgl_glBindImageTexture;
#		define glBindImageTexture fgl_glBindImageTexture
		typedef void (FGL_APIENTRY fgl_func_glMemoryBarrier)(GLbitfield barriers);
		fgl_api fgl_func_glMemoryBarrier* fgl_glMemoryBarrier;
#		define glMemoryBarrier fgl_glMemoryBarrier
		typedef void (FGL_APIENTRY fgl_func_glTexStorage1D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
		fgl_api fgl_func_glTexStorage1D* fgl_glTexStorage1D;
#		define glTexStorage1D fgl_glTexStorage1D
		typedef void (FGL_APIENTRY fgl_func_glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
		fgl_api fgl_func_glTexStorage2D* fgl_glTexStorage2D;
#		define glTexStorage2D fgl_glTexStorage2D
		typedef void (FGL_APIENTRY fgl_func_glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
		fgl_api fgl_func_glTexStorage3D* fgl_glTexStorage3D;
#		define glTexStorage3D fgl_glTexStorage3D
		typedef void (FGL_APIENTRY fgl_func_glDrawTransformFeedbackInstanced)(GLenum mode, GLuint id, GLsizei instancecount);
		fgl_api fgl_func_glDrawTransformFeedbackInstanced* fgl_glDrawTransformFeedbackInstanced;
#		define glDrawTransformFeedbackInstanced fgl_glDrawTransformFeedbackInstanced
		typedef void (FGL_APIENTRY fgl_func_glDrawTransformFeedbackStreamInstanced)(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
		fgl_api fgl_func_glDrawTransformFeedbackStreamInstanced* fgl_glDrawTransformFeedbackStreamInstanced;
#		define glDrawTransformFeedbackStreamInstanced fgl_glDrawTransformFeedbackStreamInstanced
#	endif // GL_VERSION_4_2

#	ifndef GL_VERSION_4_3
#		define GL_VERSION_4_3 1
		fgl_api bool isGL_VERSION_4_3;

		typedef void (FGL_APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

#		define GL_NUM_SHADING_LANGUAGE_VERSIONS 0x82E9
#		define GL_VERTEX_ATTRIB_ARRAY_LONG 0x874E
#		define GL_COMPRESSED_RGB8_ETC2 0x9274
#		define GL_COMPRESSED_SRGB8_ETC2 0x9275
#		define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#		define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#		define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#		define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#		define GL_COMPRESSED_R11_EAC 0x9270
#		define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
#		define GL_COMPRESSED_RG11_EAC 0x9272
#		define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
#		define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#		define GL_ANY_SAMPLES_PASSED_CONSERVATIVE 0x8D6A
#		define GL_MAX_ELEMENT_INDEX 0x8D6B
#		define GL_COMPUTE_SHADER 0x91B9
#		define GL_MAX_COMPUTE_UNIFORM_BLOCKS 0x91BB
#		define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#		define GL_MAX_COMPUTE_IMAGE_UNIFORMS 0x91BD
#		define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#		define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#		define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#		define GL_MAX_COMPUTE_ATOMIC_COUNTERS 0x8265
#		define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#		define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#		define GL_MAX_COMPUTE_WORK_GROUP_COUNT 0x91BE
#		define GL_MAX_COMPUTE_WORK_GROUP_SIZE 0x91BF
#		define GL_COMPUTE_WORK_GROUP_SIZE 0x8267
#		define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER 0x90EC
#		define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER 0x90ED
#		define GL_DISPATCH_INDIRECT_BUFFER 0x90EE
#		define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#		define GL_COMPUTE_SHADER_BIT 0x00000020
#		define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#		define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#		define GL_DEBUG_CALLBACK_FUNCTION 0x8244
#		define GL_DEBUG_CALLBACK_USER_PARAM 0x8245
#		define GL_DEBUG_SOURCE_API 0x8246
#		define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#		define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#		define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#		define GL_DEBUG_SOURCE_APPLICATION 0x824A
#		define GL_DEBUG_SOURCE_OTHER 0x824B
#		define GL_DEBUG_TYPE_ERROR 0x824C
#		define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#		define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#		define GL_DEBUG_TYPE_PORTABILITY 0x824F
#		define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#		define GL_DEBUG_TYPE_OTHER 0x8251
#		define GL_MAX_DEBUG_MESSAGE_LENGTH 0x9143
#		define GL_MAX_DEBUG_LOGGED_MESSAGES 0x9144
#		define GL_DEBUG_LOGGED_MESSAGES 0x9145
#		define GL_DEBUG_SEVERITY_HIGH 0x9146
#		define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#		define GL_DEBUG_SEVERITY_LOW 0x9148
#		define GL_DEBUG_TYPE_MARKER 0x8268
#		define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#		define GL_DEBUG_TYPE_POP_GROUP 0x826A
#		define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#		define GL_MAX_DEBUG_GROUP_STACK_DEPTH 0x826C
#		define GL_DEBUG_GROUP_STACK_DEPTH 0x826D
#		define GL_BUFFER 0x82E0
#		define GL_SHADER 0x82E1
#		define GL_PROGRAM 0x82E2
#		define GL_QUERY 0x82E3
#		define GL_PROGRAM_PIPELINE 0x82E4
#		define GL_SAMPLER 0x82E6
#		define GL_MAX_LABEL_LENGTH 0x82E8
#		define GL_DEBUG_OUTPUT 0x92E0
#		define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#		define GL_MAX_UNIFORM_LOCATIONS 0x826E
#		define GL_FRAMEBUFFER_DEFAULT_WIDTH 0x9310
#		define GL_FRAMEBUFFER_DEFAULT_HEIGHT 0x9311
#		define GL_FRAMEBUFFER_DEFAULT_LAYERS 0x9312
#		define GL_FRAMEBUFFER_DEFAULT_SAMPLES 0x9313
#		define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS 0x9314
#		define GL_MAX_FRAMEBUFFER_WIDTH 0x9315
#		define GL_MAX_FRAMEBUFFER_HEIGHT 0x9316
#		define GL_MAX_FRAMEBUFFER_LAYERS 0x9317
#		define GL_MAX_FRAMEBUFFER_SAMPLES 0x9318
#		define GL_INTERNALFORMAT_SUPPORTED 0x826F
#		define GL_INTERNALFORMAT_PREFERRED 0x8270
#		define GL_INTERNALFORMAT_RED_SIZE 0x8271
#		define GL_INTERNALFORMAT_GREEN_SIZE 0x8272
#		define GL_INTERNALFORMAT_BLUE_SIZE 0x8273
#		define GL_INTERNALFORMAT_ALPHA_SIZE 0x8274
#		define GL_INTERNALFORMAT_DEPTH_SIZE 0x8275
#		define GL_INTERNALFORMAT_STENCIL_SIZE 0x8276
#		define GL_INTERNALFORMAT_SHARED_SIZE 0x8277
#		define GL_INTERNALFORMAT_RED_TYPE 0x8278
#		define GL_INTERNALFORMAT_GREEN_TYPE 0x8279
#		define GL_INTERNALFORMAT_BLUE_TYPE 0x827A
#		define GL_INTERNALFORMAT_ALPHA_TYPE 0x827B
#		define GL_INTERNALFORMAT_DEPTH_TYPE 0x827C
#		define GL_INTERNALFORMAT_STENCIL_TYPE 0x827D
#		define GL_MAX_WIDTH 0x827E
#		define GL_MAX_HEIGHT 0x827F
#		define GL_MAX_DEPTH 0x8280
#		define GL_MAX_LAYERS 0x8281
#		define GL_MAX_COMBINED_DIMENSIONS 0x8282
#		define GL_COLOR_COMPONENTS 0x8283
#		define GL_DEPTH_COMPONENTS 0x8284
#		define GL_STENCIL_COMPONENTS 0x8285
#		define GL_COLOR_RENDERABLE 0x8286
#		define GL_DEPTH_RENDERABLE 0x8287
#		define GL_STENCIL_RENDERABLE 0x8288
#		define GL_FRAMEBUFFER_RENDERABLE 0x8289
#		define GL_FRAMEBUFFER_RENDERABLE_LAYERED 0x828A
#		define GL_FRAMEBUFFER_BLEND 0x828B
#		define GL_READ_PIXELS 0x828C
#		define GL_READ_PIXELS_FORMAT 0x828D
#		define GL_READ_PIXELS_TYPE 0x828E
#		define GL_TEXTURE_IMAGE_FORMAT 0x828F
#		define GL_TEXTURE_IMAGE_TYPE 0x8290
#		define GL_GET_TEXTURE_IMAGE_FORMAT 0x8291
#		define GL_GET_TEXTURE_IMAGE_TYPE 0x8292
#		define GL_MIPMAP 0x8293
#		define GL_MANUAL_GENERATE_MIPMAP 0x8294
#		define GL_AUTO_GENERATE_MIPMAP 0x8295
#		define GL_COLOR_ENCODING 0x8296
#		define GL_SRGB_READ 0x8297
#		define GL_SRGB_WRITE 0x8298
#		define GL_FILTER 0x829A
#		define GL_VERTEX_TEXTURE 0x829B
#		define GL_TESS_CONTROL_TEXTURE 0x829C
#		define GL_TESS_EVALUATION_TEXTURE 0x829D
#		define GL_GEOMETRY_TEXTURE 0x829E
#		define GL_FRAGMENT_TEXTURE 0x829F
#		define GL_COMPUTE_TEXTURE 0x82A0
#		define GL_TEXTURE_SHADOW 0x82A1
#		define GL_TEXTURE_GATHER 0x82A2
#		define GL_TEXTURE_GATHER_SHADOW 0x82A3
#		define GL_SHADER_IMAGE_LOAD 0x82A4
#		define GL_SHADER_IMAGE_STORE 0x82A5
#		define GL_SHADER_IMAGE_ATOMIC 0x82A6
#		define GL_IMAGE_TEXEL_SIZE 0x82A7
#		define GL_IMAGE_COMPATIBILITY_CLASS 0x82A8
#		define GL_IMAGE_PIXEL_FORMAT 0x82A9
#		define GL_IMAGE_PIXEL_TYPE 0x82AA
#		define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST 0x82AC
#		define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST 0x82AD
#		define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE 0x82AE
#		define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE 0x82AF
#		define GL_TEXTURE_COMPRESSED_BLOCK_WIDTH 0x82B1
#		define GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT 0x82B2
#		define GL_TEXTURE_COMPRESSED_BLOCK_SIZE 0x82B3
#		define GL_CLEAR_BUFFER 0x82B4
#		define GL_TEXTURE_VIEW 0x82B5
#		define GL_VIEW_COMPATIBILITY_CLASS 0x82B6
#		define GL_FULL_SUPPORT 0x82B7
#		define GL_CAVEAT_SUPPORT 0x82B8
#		define GL_IMAGE_CLASS_4_X_32 0x82B9
#		define GL_IMAGE_CLASS_2_X_32 0x82BA
#		define GL_IMAGE_CLASS_1_X_32 0x82BB
#		define GL_IMAGE_CLASS_4_X_16 0x82BC
#		define GL_IMAGE_CLASS_2_X_16 0x82BD
#		define GL_IMAGE_CLASS_1_X_16 0x82BE
#		define GL_IMAGE_CLASS_4_X_8 0x82BF
#		define GL_IMAGE_CLASS_2_X_8 0x82C0
#		define GL_IMAGE_CLASS_1_X_8 0x82C1
#		define GL_IMAGE_CLASS_11_11_10 0x82C2
#		define GL_IMAGE_CLASS_10_10_10_2 0x82C3
#		define GL_VIEW_CLASS_128_BITS 0x82C4
#		define GL_VIEW_CLASS_96_BITS 0x82C5
#		define GL_VIEW_CLASS_64_BITS 0x82C6
#		define GL_VIEW_CLASS_48_BITS 0x82C7
#		define GL_VIEW_CLASS_32_BITS 0x82C8
#		define GL_VIEW_CLASS_24_BITS 0x82C9
#		define GL_VIEW_CLASS_16_BITS 0x82CA
#		define GL_VIEW_CLASS_8_BITS 0x82CB
#		define GL_VIEW_CLASS_S3TC_DXT1_RGB 0x82CC
#		define GL_VIEW_CLASS_S3TC_DXT1_RGBA 0x82CD
#		define GL_VIEW_CLASS_S3TC_DXT3_RGBA 0x82CE
#		define GL_VIEW_CLASS_S3TC_DXT5_RGBA 0x82CF
#		define GL_VIEW_CLASS_RGTC1_RED 0x82D0
#		define GL_VIEW_CLASS_RGTC2_RG 0x82D1
#		define GL_VIEW_CLASS_BPTC_UNORM 0x82D2
#		define GL_VIEW_CLASS_BPTC_FLOAT 0x82D3
#		define GL_UNIFORM 0x92E1
#		define GL_UNIFORM_BLOCK 0x92E2
#		define GL_PROGRAM_INPUT 0x92E3
#		define GL_PROGRAM_OUTPUT 0x92E4
#		define GL_BUFFER_VARIABLE 0x92E5
#		define GL_SHADER_STORAGE_BLOCK 0x92E6
#		define GL_VERTEX_SUBROUTINE 0x92E8
#		define GL_TESS_CONTROL_SUBROUTINE 0x92E9
#		define GL_TESS_EVALUATION_SUBROUTINE 0x92EA
#		define GL_GEOMETRY_SUBROUTINE 0x92EB
#		define GL_FRAGMENT_SUBROUTINE 0x92EC
#		define GL_COMPUTE_SUBROUTINE 0x92ED
#		define GL_VERTEX_SUBROUTINE_UNIFORM 0x92EE
#		define GL_TESS_CONTROL_SUBROUTINE_UNIFORM 0x92EF
#		define GL_TESS_EVALUATION_SUBROUTINE_UNIFORM 0x92F0
#		define GL_GEOMETRY_SUBROUTINE_UNIFORM 0x92F1
#		define GL_FRAGMENT_SUBROUTINE_UNIFORM 0x92F2
#		define GL_COMPUTE_SUBROUTINE_UNIFORM 0x92F3
#		define GL_TRANSFORM_FEEDBACK_VARYING 0x92F4
#		define GL_ACTIVE_RESOURCES 0x92F5
#		define GL_MAX_NAME_LENGTH 0x92F6
#		define GL_MAX_NUM_ACTIVE_VARIABLES 0x92F7
#		define GL_MAX_NUM_COMPATIBLE_SUBROUTINES 0x92F8
#		define GL_NAME_LENGTH 0x92F9
#		define GL_TYPE 0x92FA
#		define GL_ARRAY_SIZE 0x92FB
#		define GL_OFFSET 0x92FC
#		define GL_BLOCK_INDEX 0x92FD
#		define GL_ARRAY_STRIDE 0x92FE
#		define GL_MATRIX_STRIDE 0x92FF
#		define GL_IS_ROW_MAJOR 0x9300
#		define GL_ATOMIC_COUNTER_BUFFER_INDEX 0x9301
#		define GL_BUFFER_BINDING 0x9302
#		define GL_BUFFER_DATA_SIZE 0x9303
#		define GL_NUM_ACTIVE_VARIABLES 0x9304
#		define GL_ACTIVE_VARIABLES 0x9305
#		define GL_REFERENCED_BY_VERTEX_SHADER 0x9306
#		define GL_REFERENCED_BY_TESS_CONTROL_SHADER 0x9307
#		define GL_REFERENCED_BY_TESS_EVALUATION_SHADER 0x9308
#		define GL_REFERENCED_BY_GEOMETRY_SHADER 0x9309
#		define GL_REFERENCED_BY_FRAGMENT_SHADER 0x930A
#		define GL_REFERENCED_BY_COMPUTE_SHADER 0x930B
#		define GL_TOP_LEVEL_ARRAY_SIZE 0x930C
#		define GL_TOP_LEVEL_ARRAY_STRIDE 0x930D
#		define GL_LOCATION 0x930E
#		define GL_LOCATION_INDEX 0x930F
#		define GL_IS_PER_PATCH 0x92E7
#		define GL_SHADER_STORAGE_BUFFER 0x90D2
#		define GL_SHADER_STORAGE_BUFFER_BINDING 0x90D3
#		define GL_SHADER_STORAGE_BUFFER_START 0x90D4
#		define GL_SHADER_STORAGE_BUFFER_SIZE 0x90D5
#		define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#		define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS 0x90D7
#		define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS 0x90D8
#		define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS 0x90D9
#		define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#		define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#		define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#		define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#		define GL_MAX_SHADER_STORAGE_BLOCK_SIZE 0x90DE
#		define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#		define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#		define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#		define GL_DEPTH_STENCIL_TEXTURE_MODE 0x90EA
#		define GL_TEXTURE_BUFFER_OFFSET 0x919D
#		define GL_TEXTURE_BUFFER_SIZE 0x919E
#		define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT 0x919F
#		define GL_TEXTURE_VIEW_MIN_LEVEL 0x82DB
#		define GL_TEXTURE_VIEW_NUM_LEVELS 0x82DC
#		define GL_TEXTURE_VIEW_MIN_LAYER 0x82DD
#		define GL_TEXTURE_VIEW_NUM_LAYERS 0x82DE
#		define GL_TEXTURE_IMMUTABLE_LEVELS 0x82DF
#		define GL_VERTEX_ATTRIB_BINDING 0x82D4
#		define GL_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D5
#		define GL_VERTEX_BINDING_DIVISOR 0x82D6
#		define GL_VERTEX_BINDING_OFFSET 0x82D7
#		define GL_VERTEX_BINDING_STRIDE 0x82D8
#		define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#		define GL_MAX_VERTEX_ATTRIB_BINDINGS 0x82DA
#		define GL_VERTEX_BINDING_BUFFER 0x8F4F
#		define GL_DISPLAY_LIST 0x82E7

		typedef void (FGL_APIENTRY fgl_func_glClearBufferData)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);
		fgl_api fgl_func_glClearBufferData* fgl_glClearBufferData;
#		define glClearBufferData fgl_glClearBufferData
		typedef void (FGL_APIENTRY fgl_func_glClearBufferSubData)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
		fgl_api fgl_func_glClearBufferSubData* fgl_glClearBufferSubData;
#		define glClearBufferSubData fgl_glClearBufferSubData
		typedef void (FGL_APIENTRY fgl_func_glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
		fgl_api fgl_func_glDispatchCompute* fgl_glDispatchCompute;
#		define glDispatchCompute fgl_glDispatchCompute
		typedef void (FGL_APIENTRY fgl_func_glDispatchComputeIndirect)(GLintptr indirect);
		fgl_api fgl_func_glDispatchComputeIndirect* fgl_glDispatchComputeIndirect;
#		define glDispatchComputeIndirect fgl_glDispatchComputeIndirect
		typedef void (FGL_APIENTRY fgl_func_glCopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
		fgl_api fgl_func_glCopyImageSubData* fgl_glCopyImageSubData;
#		define glCopyImageSubData fgl_glCopyImageSubData
		typedef void (FGL_APIENTRY fgl_func_glFramebufferParameteri)(GLenum target, GLenum pname, GLint param);
		fgl_api fgl_func_glFramebufferParameteri* fgl_glFramebufferParameteri;
#		define glFramebufferParameteri fgl_glFramebufferParameteri
		typedef void (FGL_APIENTRY fgl_func_glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetFramebufferParameteriv* fgl_glGetFramebufferParameteriv;
#		define glGetFramebufferParameteriv fgl_glGetFramebufferParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGetInternalformati64v)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
		fgl_api fgl_func_glGetInternalformati64v* fgl_glGetInternalformati64v;
#		define glGetInternalformati64v fgl_glGetInternalformati64v
		typedef void (FGL_APIENTRY fgl_func_glInvalidateTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
		fgl_api fgl_func_glInvalidateTexSubImage* fgl_glInvalidateTexSubImage;
#		define glInvalidateTexSubImage fgl_glInvalidateTexSubImage
		typedef void (FGL_APIENTRY fgl_func_glInvalidateTexImage)(GLuint texture, GLint level);
		fgl_api fgl_func_glInvalidateTexImage* fgl_glInvalidateTexImage;
#		define glInvalidateTexImage fgl_glInvalidateTexImage
		typedef void (FGL_APIENTRY fgl_func_glInvalidateBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr length);
		fgl_api fgl_func_glInvalidateBufferSubData* fgl_glInvalidateBufferSubData;
#		define glInvalidateBufferSubData fgl_glInvalidateBufferSubData
		typedef void (FGL_APIENTRY fgl_func_glInvalidateBufferData)(GLuint buffer);
		fgl_api fgl_func_glInvalidateBufferData* fgl_glInvalidateBufferData;
#		define glInvalidateBufferData fgl_glInvalidateBufferData
		typedef void (FGL_APIENTRY fgl_func_glInvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
		fgl_api fgl_func_glInvalidateFramebuffer* fgl_glInvalidateFramebuffer;
#		define glInvalidateFramebuffer fgl_glInvalidateFramebuffer
		typedef void (FGL_APIENTRY fgl_func_glInvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glInvalidateSubFramebuffer* fgl_glInvalidateSubFramebuffer;
#		define glInvalidateSubFramebuffer fgl_glInvalidateSubFramebuffer
		typedef void (FGL_APIENTRY fgl_func_glMultiDrawArraysIndirect)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
		fgl_api fgl_func_glMultiDrawArraysIndirect* fgl_glMultiDrawArraysIndirect;
#		define glMultiDrawArraysIndirect fgl_glMultiDrawArraysIndirect
		typedef void (FGL_APIENTRY fgl_func_glMultiDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
		fgl_api fgl_func_glMultiDrawElementsIndirect* fgl_glMultiDrawElementsIndirect;
#		define glMultiDrawElementsIndirect fgl_glMultiDrawElementsIndirect
		typedef void (FGL_APIENTRY fgl_func_glGetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetProgramInterfaceiv* fgl_glGetProgramInterfaceiv;
#		define glGetProgramInterfaceiv fgl_glGetProgramInterfaceiv
		typedef GLuint (FGL_APIENTRY fgl_func_glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name);
		fgl_api fgl_func_glGetProgramResourceIndex* fgl_glGetProgramResourceIndex;
#		define glGetProgramResourceIndex fgl_glGetProgramResourceIndex
		typedef void (FGL_APIENTRY fgl_func_glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
		fgl_api fgl_func_glGetProgramResourceName* fgl_glGetProgramResourceName;
#		define glGetProgramResourceName fgl_glGetProgramResourceName
		typedef void (FGL_APIENTRY fgl_func_glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
		fgl_api fgl_func_glGetProgramResourceiv* fgl_glGetProgramResourceiv;
#		define glGetProgramResourceiv fgl_glGetProgramResourceiv
		typedef GLint (FGL_APIENTRY fgl_func_glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name);
		fgl_api fgl_func_glGetProgramResourceLocation* fgl_glGetProgramResourceLocation;
#		define glGetProgramResourceLocation fgl_glGetProgramResourceLocation
		typedef GLint (FGL_APIENTRY fgl_func_glGetProgramResourceLocationIndex)(GLuint program, GLenum programInterface, const GLchar *name);
		fgl_api fgl_func_glGetProgramResourceLocationIndex* fgl_glGetProgramResourceLocationIndex;
#		define glGetProgramResourceLocationIndex fgl_glGetProgramResourceLocationIndex
		typedef void (FGL_APIENTRY fgl_func_glShaderStorageBlockBinding)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
		fgl_api fgl_func_glShaderStorageBlockBinding* fgl_glShaderStorageBlockBinding;
#		define glShaderStorageBlockBinding fgl_glShaderStorageBlockBinding
		typedef void (FGL_APIENTRY fgl_func_glTexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
		fgl_api fgl_func_glTexBufferRange* fgl_glTexBufferRange;
#		define glTexBufferRange fgl_glTexBufferRange
		typedef void (FGL_APIENTRY fgl_func_glTexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		fgl_api fgl_func_glTexStorage2DMultisample* fgl_glTexStorage2DMultisample;
#		define glTexStorage2DMultisample fgl_glTexStorage2DMultisample
		typedef void (FGL_APIENTRY fgl_func_glTexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		fgl_api fgl_func_glTexStorage3DMultisample* fgl_glTexStorage3DMultisample;
#		define glTexStorage3DMultisample fgl_glTexStorage3DMultisample
		typedef void (FGL_APIENTRY fgl_func_glTextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
		fgl_api fgl_func_glTextureView* fgl_glTextureView;
#		define glTextureView fgl_glTextureView
		typedef void (FGL_APIENTRY fgl_func_glBindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
		fgl_api fgl_func_glBindVertexBuffer* fgl_glBindVertexBuffer;
#		define glBindVertexBuffer fgl_glBindVertexBuffer
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
		fgl_api fgl_func_glVertexAttribFormat* fgl_glVertexAttribFormat;
#		define glVertexAttribFormat fgl_glVertexAttribFormat
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		fgl_api fgl_func_glVertexAttribIFormat* fgl_glVertexAttribIFormat;
#		define glVertexAttribIFormat fgl_glVertexAttribIFormat
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribLFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		fgl_api fgl_func_glVertexAttribLFormat* fgl_glVertexAttribLFormat;
#		define glVertexAttribLFormat fgl_glVertexAttribLFormat
		typedef void (FGL_APIENTRY fgl_func_glVertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
		fgl_api fgl_func_glVertexAttribBinding* fgl_glVertexAttribBinding;
#		define glVertexAttribBinding fgl_glVertexAttribBinding
		typedef void (FGL_APIENTRY fgl_func_glVertexBindingDivisor)(GLuint bindingindex, GLuint divisor);
		fgl_api fgl_func_glVertexBindingDivisor* fgl_glVertexBindingDivisor;
#		define glVertexBindingDivisor fgl_glVertexBindingDivisor
		typedef void (FGL_APIENTRY fgl_func_glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
		fgl_api fgl_func_glDebugMessageControl* fgl_glDebugMessageControl;
#		define glDebugMessageControl fgl_glDebugMessageControl
		typedef void (FGL_APIENTRY fgl_func_glDebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
		fgl_api fgl_func_glDebugMessageInsert* fgl_glDebugMessageInsert;
#		define glDebugMessageInsert fgl_glDebugMessageInsert
		typedef void (FGL_APIENTRY fgl_func_glDebugMessageCallback)(GLDEBUGPROC callback, const void *userParam);
		fgl_api fgl_func_glDebugMessageCallback* fgl_glDebugMessageCallback;
#		define glDebugMessageCallback fgl_glDebugMessageCallback
		typedef GLuint (FGL_APIENTRY fgl_func_glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
		fgl_api fgl_func_glGetDebugMessageLog* fgl_glGetDebugMessageLog;
#		define glGetDebugMessageLog fgl_glGetDebugMessageLog
		typedef void (FGL_APIENTRY fgl_func_glPushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar *message);
		fgl_api fgl_func_glPushDebugGroup* fgl_glPushDebugGroup;
#		define glPushDebugGroup fgl_glPushDebugGroup
		typedef void (FGL_APIENTRY fgl_func_glPopDebugGroup)(void);
		fgl_api fgl_func_glPopDebugGroup* fgl_glPopDebugGroup;
#		define glPopDebugGroup fgl_glPopDebugGroup
		typedef void (FGL_APIENTRY fgl_func_glObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
		fgl_api fgl_func_glObjectLabel* fgl_glObjectLabel;
#		define glObjectLabel fgl_glObjectLabel
		typedef void (FGL_APIENTRY fgl_func_glGetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
		fgl_api fgl_func_glGetObjectLabel* fgl_glGetObjectLabel;
#		define glGetObjectLabel fgl_glGetObjectLabel
		typedef void (FGL_APIENTRY fgl_func_glObjectPtrLabel)(const void *ptr, GLsizei length, const GLchar *label);
		fgl_api fgl_func_glObjectPtrLabel* fgl_glObjectPtrLabel;
#		define glObjectPtrLabel fgl_glObjectPtrLabel
		typedef void (FGL_APIENTRY fgl_func_glGetObjectPtrLabel)(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
		fgl_api fgl_func_glGetObjectPtrLabel* fgl_glGetObjectPtrLabel;
#		define glGetObjectPtrLabel fgl_glGetObjectPtrLabel
#	endif // GL_VERSION_4_3

#	ifndef GL_VERSION_4_4
#		define GL_VERSION_4_4 1
		fgl_api bool isGL_VERSION_4_4;

#		define GL_MAX_VERTEX_ATTRIB_STRIDE 0x82E5
#		define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED 0x8221
#		define GL_TEXTURE_BUFFER_BINDING 0x8C2A
#		define GL_MAP_PERSISTENT_BIT 0x0040
#		define GL_MAP_COHERENT_BIT 0x0080
#		define GL_DYNAMIC_STORAGE_BIT 0x0100
#		define GL_CLIENT_STORAGE_BIT 0x0200
#		define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT 0x00004000
#		define GL_BUFFER_IMMUTABLE_STORAGE 0x821F
#		define GL_BUFFER_STORAGE_FLAGS 0x8220
#		define GL_CLEAR_TEXTURE 0x9365
#		define GL_LOCATION_COMPONENT 0x934A
#		define GL_TRANSFORM_FEEDBACK_BUFFER_INDEX 0x934B
#		define GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE 0x934C
#		define GL_QUERY_BUFFER 0x9192
#		define GL_QUERY_BUFFER_BARRIER_BIT 0x00008000
#		define GL_QUERY_BUFFER_BINDING 0x9193
#		define GL_QUERY_RESULT_NO_WAIT 0x9194
#		define GL_MIRROR_CLAMP_TO_EDGE 0x8743

		typedef void (FGL_APIENTRY fgl_func_glBufferStorage)(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
		fgl_api fgl_func_glBufferStorage* fgl_glBufferStorage;
#		define glBufferStorage fgl_glBufferStorage
		typedef void (FGL_APIENTRY fgl_func_glClearTexImage)(GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
		fgl_api fgl_func_glClearTexImage* fgl_glClearTexImage;
#		define glClearTexImage fgl_glClearTexImage
		typedef void (FGL_APIENTRY fgl_func_glClearTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
		fgl_api fgl_func_glClearTexSubImage* fgl_glClearTexSubImage;
#		define glClearTexSubImage fgl_glClearTexSubImage
		typedef void (FGL_APIENTRY fgl_func_glBindBuffersBase)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
		fgl_api fgl_func_glBindBuffersBase* fgl_glBindBuffersBase;
#		define glBindBuffersBase fgl_glBindBuffersBase
		typedef void (FGL_APIENTRY fgl_func_glBindBuffersRange)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
		fgl_api fgl_func_glBindBuffersRange* fgl_glBindBuffersRange;
#		define glBindBuffersRange fgl_glBindBuffersRange
		typedef void (FGL_APIENTRY fgl_func_glBindTextures)(GLuint first, GLsizei count, const GLuint *textures);
		fgl_api fgl_func_glBindTextures* fgl_glBindTextures;
#		define glBindTextures fgl_glBindTextures
		typedef void (FGL_APIENTRY fgl_func_glBindSamplers)(GLuint first, GLsizei count, const GLuint *samplers);
		fgl_api fgl_func_glBindSamplers* fgl_glBindSamplers;
#		define glBindSamplers fgl_glBindSamplers
		typedef void (FGL_APIENTRY fgl_func_glBindImageTextures)(GLuint first, GLsizei count, const GLuint *textures);
		fgl_api fgl_func_glBindImageTextures* fgl_glBindImageTextures;
#		define glBindImageTextures fgl_glBindImageTextures
		typedef void (FGL_APIENTRY fgl_func_glBindVertexBuffers)(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
		fgl_api fgl_func_glBindVertexBuffers* fgl_glBindVertexBuffers;
#		define glBindVertexBuffers fgl_glBindVertexBuffers
#	endif // GL_VERSION_4_4

#	ifndef GL_VERSION_4_5
#		define GL_VERSION_4_5 1
		fgl_api bool isGL_VERSION_4_5;

#		define GL_CONTEXT_LOST 0x0507
#		define GL_NEGATIVE_ONE_TO_ONE 0x935E
#		define GL_ZERO_TO_ONE 0x935F
#		define GL_CLIP_ORIGIN 0x935C
#		define GL_CLIP_DEPTH_MODE 0x935D
#		define GL_QUERY_WAIT_INVERTED 0x8E17
#		define GL_QUERY_NO_WAIT_INVERTED 0x8E18
#		define GL_QUERY_BY_REGION_WAIT_INVERTED 0x8E19
#		define GL_QUERY_BY_REGION_NO_WAIT_INVERTED 0x8E1A
#		define GL_MAX_CULL_DISTANCES 0x82F9
#		define GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES 0x82FA
#		define GL_TEXTURE_TARGET 0x1006
#		define GL_QUERY_TARGET 0x82EA
#		define GL_GUILTY_CONTEXT_RESET 0x8253
#		define GL_INNOCENT_CONTEXT_RESET 0x8254
#		define GL_UNKNOWN_CONTEXT_RESET 0x8255
#		define GL_RESET_NOTIFICATION_STRATEGY 0x8256
#		define GL_LOSE_CONTEXT_ON_RESET 0x8252
#		define GL_NO_RESET_NOTIFICATION 0x8261
#		define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#		define GL_CONTEXT_RELEASE_BEHAVIOR 0x82FB
#		define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82FC

		typedef void (FGL_APIENTRY fgl_func_glClipControl)(GLenum origin, GLenum depth);
		fgl_api fgl_func_glClipControl* fgl_glClipControl;
#		define glClipControl fgl_glClipControl
		typedef void (FGL_APIENTRY fgl_func_glCreateTransformFeedbacks)(GLsizei n, GLuint *ids);
		fgl_api fgl_func_glCreateTransformFeedbacks* fgl_glCreateTransformFeedbacks;
#		define glCreateTransformFeedbacks fgl_glCreateTransformFeedbacks
		typedef void (FGL_APIENTRY fgl_func_glTransformFeedbackBufferBase)(GLuint xfb, GLuint index, GLuint buffer);
		fgl_api fgl_func_glTransformFeedbackBufferBase* fgl_glTransformFeedbackBufferBase;
#		define glTransformFeedbackBufferBase fgl_glTransformFeedbackBufferBase
		typedef void (FGL_APIENTRY fgl_func_glTransformFeedbackBufferRange)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
		fgl_api fgl_func_glTransformFeedbackBufferRange* fgl_glTransformFeedbackBufferRange;
#		define glTransformFeedbackBufferRange fgl_glTransformFeedbackBufferRange
		typedef void (FGL_APIENTRY fgl_func_glGetTransformFeedbackiv)(GLuint xfb, GLenum pname, GLint *param);
		fgl_api fgl_func_glGetTransformFeedbackiv* fgl_glGetTransformFeedbackiv;
#		define glGetTransformFeedbackiv fgl_glGetTransformFeedbackiv
		typedef void (FGL_APIENTRY fgl_func_glGetTransformFeedbacki_v)(GLuint xfb, GLenum pname, GLuint index, GLint *param);
		fgl_api fgl_func_glGetTransformFeedbacki_v* fgl_glGetTransformFeedbacki_v;
#		define glGetTransformFeedbacki_v fgl_glGetTransformFeedbacki_v
		typedef void (FGL_APIENTRY fgl_func_glGetTransformFeedbacki64_v)(GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
		fgl_api fgl_func_glGetTransformFeedbacki64_v* fgl_glGetTransformFeedbacki64_v;
#		define glGetTransformFeedbacki64_v fgl_glGetTransformFeedbacki64_v
		typedef void (FGL_APIENTRY fgl_func_glCreateBuffers)(GLsizei n, GLuint *buffers);
		fgl_api fgl_func_glCreateBuffers* fgl_glCreateBuffers;
#		define glCreateBuffers fgl_glCreateBuffers
		typedef void (FGL_APIENTRY fgl_func_glNamedBufferStorage)(GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
		fgl_api fgl_func_glNamedBufferStorage* fgl_glNamedBufferStorage;
#		define glNamedBufferStorage fgl_glNamedBufferStorage
		typedef void (FGL_APIENTRY fgl_func_glNamedBufferData)(GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
		fgl_api fgl_func_glNamedBufferData* fgl_glNamedBufferData;
#		define glNamedBufferData fgl_glNamedBufferData
		typedef void (FGL_APIENTRY fgl_func_glNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
		fgl_api fgl_func_glNamedBufferSubData* fgl_glNamedBufferSubData;
#		define glNamedBufferSubData fgl_glNamedBufferSubData
		typedef void (FGL_APIENTRY fgl_func_glCopyNamedBufferSubData)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
		fgl_api fgl_func_glCopyNamedBufferSubData* fgl_glCopyNamedBufferSubData;
#		define glCopyNamedBufferSubData fgl_glCopyNamedBufferSubData
		typedef void (FGL_APIENTRY fgl_func_glClearNamedBufferData)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
		fgl_api fgl_func_glClearNamedBufferData* fgl_glClearNamedBufferData;
#		define glClearNamedBufferData fgl_glClearNamedBufferData
		typedef void (FGL_APIENTRY fgl_func_glClearNamedBufferSubData)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
		fgl_api fgl_func_glClearNamedBufferSubData* fgl_glClearNamedBufferSubData;
#		define glClearNamedBufferSubData fgl_glClearNamedBufferSubData
		typedef void * (FGL_APIENTRY fgl_func_glMapNamedBuffer)(GLuint buffer, GLenum access);
		fgl_api fgl_func_glMapNamedBuffer* fgl_glMapNamedBuffer;
#		define glMapNamedBuffer fgl_glMapNamedBuffer
		typedef void * (FGL_APIENTRY fgl_func_glMapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
		fgl_api fgl_func_glMapNamedBufferRange* fgl_glMapNamedBufferRange;
#		define glMapNamedBufferRange fgl_glMapNamedBufferRange
		typedef GLboolean (FGL_APIENTRY fgl_func_glUnmapNamedBuffer)(GLuint buffer);
		fgl_api fgl_func_glUnmapNamedBuffer* fgl_glUnmapNamedBuffer;
#		define glUnmapNamedBuffer fgl_glUnmapNamedBuffer
		typedef void (FGL_APIENTRY fgl_func_glFlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length);
		fgl_api fgl_func_glFlushMappedNamedBufferRange* fgl_glFlushMappedNamedBufferRange;
#		define glFlushMappedNamedBufferRange fgl_glFlushMappedNamedBufferRange
		typedef void (FGL_APIENTRY fgl_func_glGetNamedBufferParameteriv)(GLuint buffer, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetNamedBufferParameteriv* fgl_glGetNamedBufferParameteriv;
#		define glGetNamedBufferParameteriv fgl_glGetNamedBufferParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGetNamedBufferParameteri64v)(GLuint buffer, GLenum pname, GLint64 *params);
		fgl_api fgl_func_glGetNamedBufferParameteri64v* fgl_glGetNamedBufferParameteri64v;
#		define glGetNamedBufferParameteri64v fgl_glGetNamedBufferParameteri64v
		typedef void (FGL_APIENTRY fgl_func_glGetNamedBufferPointerv)(GLuint buffer, GLenum pname, void **params);
		fgl_api fgl_func_glGetNamedBufferPointerv* fgl_glGetNamedBufferPointerv;
#		define glGetNamedBufferPointerv fgl_glGetNamedBufferPointerv
		typedef void (FGL_APIENTRY fgl_func_glGetNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, void *data);
		fgl_api fgl_func_glGetNamedBufferSubData* fgl_glGetNamedBufferSubData;
#		define glGetNamedBufferSubData fgl_glGetNamedBufferSubData
		typedef void (FGL_APIENTRY fgl_func_glCreateFramebuffers)(GLsizei n, GLuint *framebuffers);
		fgl_api fgl_func_glCreateFramebuffers* fgl_glCreateFramebuffers;
#		define glCreateFramebuffers fgl_glCreateFramebuffers
		typedef void (FGL_APIENTRY fgl_func_glNamedFramebufferRenderbuffer)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		fgl_api fgl_func_glNamedFramebufferRenderbuffer* fgl_glNamedFramebufferRenderbuffer;
#		define glNamedFramebufferRenderbuffer fgl_glNamedFramebufferRenderbuffer
		typedef void (FGL_APIENTRY fgl_func_glNamedFramebufferParameteri)(GLuint framebuffer, GLenum pname, GLint param);
		fgl_api fgl_func_glNamedFramebufferParameteri* fgl_glNamedFramebufferParameteri;
#		define glNamedFramebufferParameteri fgl_glNamedFramebufferParameteri
		typedef void (FGL_APIENTRY fgl_func_glNamedFramebufferTexture)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
		fgl_api fgl_func_glNamedFramebufferTexture* fgl_glNamedFramebufferTexture;
#		define glNamedFramebufferTexture fgl_glNamedFramebufferTexture
		typedef void (FGL_APIENTRY fgl_func_glNamedFramebufferTextureLayer)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
		fgl_api fgl_func_glNamedFramebufferTextureLayer* fgl_glNamedFramebufferTextureLayer;
#		define glNamedFramebufferTextureLayer fgl_glNamedFramebufferTextureLayer
		typedef void (FGL_APIENTRY fgl_func_glNamedFramebufferDrawBuffer)(GLuint framebuffer, GLenum buf);
		fgl_api fgl_func_glNamedFramebufferDrawBuffer* fgl_glNamedFramebufferDrawBuffer;
#		define glNamedFramebufferDrawBuffer fgl_glNamedFramebufferDrawBuffer
		typedef void (FGL_APIENTRY fgl_func_glNamedFramebufferDrawBuffers)(GLuint framebuffer, GLsizei n, const GLenum *bufs);
		fgl_api fgl_func_glNamedFramebufferDrawBuffers* fgl_glNamedFramebufferDrawBuffers;
#		define glNamedFramebufferDrawBuffers fgl_glNamedFramebufferDrawBuffers
		typedef void (FGL_APIENTRY fgl_func_glNamedFramebufferReadBuffer)(GLuint framebuffer, GLenum src);
		fgl_api fgl_func_glNamedFramebufferReadBuffer* fgl_glNamedFramebufferReadBuffer;
#		define glNamedFramebufferReadBuffer fgl_glNamedFramebufferReadBuffer
		typedef void (FGL_APIENTRY fgl_func_glInvalidateNamedFramebufferData)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
		fgl_api fgl_func_glInvalidateNamedFramebufferData* fgl_glInvalidateNamedFramebufferData;
#		define glInvalidateNamedFramebufferData fgl_glInvalidateNamedFramebufferData
		typedef void (FGL_APIENTRY fgl_func_glInvalidateNamedFramebufferSubData)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glInvalidateNamedFramebufferSubData* fgl_glInvalidateNamedFramebufferSubData;
#		define glInvalidateNamedFramebufferSubData fgl_glInvalidateNamedFramebufferSubData
		typedef void (FGL_APIENTRY fgl_func_glClearNamedFramebufferiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
		fgl_api fgl_func_glClearNamedFramebufferiv* fgl_glClearNamedFramebufferiv;
#		define glClearNamedFramebufferiv fgl_glClearNamedFramebufferiv
		typedef void (FGL_APIENTRY fgl_func_glClearNamedFramebufferuiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
		fgl_api fgl_func_glClearNamedFramebufferuiv* fgl_glClearNamedFramebufferuiv;
#		define glClearNamedFramebufferuiv fgl_glClearNamedFramebufferuiv
		typedef void (FGL_APIENTRY fgl_func_glClearNamedFramebufferfv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
		fgl_api fgl_func_glClearNamedFramebufferfv* fgl_glClearNamedFramebufferfv;
#		define glClearNamedFramebufferfv fgl_glClearNamedFramebufferfv
		typedef void (FGL_APIENTRY fgl_func_glClearNamedFramebufferfi)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
		fgl_api fgl_func_glClearNamedFramebufferfi* fgl_glClearNamedFramebufferfi;
#		define glClearNamedFramebufferfi fgl_glClearNamedFramebufferfi
		typedef void (FGL_APIENTRY fgl_func_glBlitNamedFramebuffer)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
		fgl_api fgl_func_glBlitNamedFramebuffer* fgl_glBlitNamedFramebuffer;
#		define glBlitNamedFramebuffer fgl_glBlitNamedFramebuffer
		typedef GLenum (FGL_APIENTRY fgl_func_glCheckNamedFramebufferStatus)(GLuint framebuffer, GLenum target);
		fgl_api fgl_func_glCheckNamedFramebufferStatus* fgl_glCheckNamedFramebufferStatus;
#		define glCheckNamedFramebufferStatus fgl_glCheckNamedFramebufferStatus
		typedef void (FGL_APIENTRY fgl_func_glGetNamedFramebufferParameteriv)(GLuint framebuffer, GLenum pname, GLint *param);
		fgl_api fgl_func_glGetNamedFramebufferParameteriv* fgl_glGetNamedFramebufferParameteriv;
#		define glGetNamedFramebufferParameteriv fgl_glGetNamedFramebufferParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGetNamedFramebufferAttachmentParameteriv)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetNamedFramebufferAttachmentParameteriv* fgl_glGetNamedFramebufferAttachmentParameteriv;
#		define glGetNamedFramebufferAttachmentParameteriv fgl_glGetNamedFramebufferAttachmentParameteriv
		typedef void (FGL_APIENTRY fgl_func_glCreateRenderbuffers)(GLsizei n, GLuint *renderbuffers);
		fgl_api fgl_func_glCreateRenderbuffers* fgl_glCreateRenderbuffers;
#		define glCreateRenderbuffers fgl_glCreateRenderbuffers
		typedef void (FGL_APIENTRY fgl_func_glNamedRenderbufferStorage)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
		fgl_api fgl_func_glNamedRenderbufferStorage* fgl_glNamedRenderbufferStorage;
#		define glNamedRenderbufferStorage fgl_glNamedRenderbufferStorage
		typedef void (FGL_APIENTRY fgl_func_glNamedRenderbufferStorageMultisample)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
		fgl_api fgl_func_glNamedRenderbufferStorageMultisample* fgl_glNamedRenderbufferStorageMultisample;
#		define glNamedRenderbufferStorageMultisample fgl_glNamedRenderbufferStorageMultisample
		typedef void (FGL_APIENTRY fgl_func_glGetNamedRenderbufferParameteriv)(GLuint renderbuffer, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetNamedRenderbufferParameteriv* fgl_glGetNamedRenderbufferParameteriv;
#		define glGetNamedRenderbufferParameteriv fgl_glGetNamedRenderbufferParameteriv
		typedef void (FGL_APIENTRY fgl_func_glCreateTextures)(GLenum target, GLsizei n, GLuint *textures);
		fgl_api fgl_func_glCreateTextures* fgl_glCreateTextures;
#		define glCreateTextures fgl_glCreateTextures
		typedef void (FGL_APIENTRY fgl_func_glTextureBuffer)(GLuint texture, GLenum internalformat, GLuint buffer);
		fgl_api fgl_func_glTextureBuffer* fgl_glTextureBuffer;
#		define glTextureBuffer fgl_glTextureBuffer
		typedef void (FGL_APIENTRY fgl_func_glTextureBufferRange)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
		fgl_api fgl_func_glTextureBufferRange* fgl_glTextureBufferRange;
#		define glTextureBufferRange fgl_glTextureBufferRange
		typedef void (FGL_APIENTRY fgl_func_glTextureStorage1D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
		fgl_api fgl_func_glTextureStorage1D* fgl_glTextureStorage1D;
#		define glTextureStorage1D fgl_glTextureStorage1D
		typedef void (FGL_APIENTRY fgl_func_glTextureStorage2D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
		fgl_api fgl_func_glTextureStorage2D* fgl_glTextureStorage2D;
#		define glTextureStorage2D fgl_glTextureStorage2D
		typedef void (FGL_APIENTRY fgl_func_glTextureStorage3D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
		fgl_api fgl_func_glTextureStorage3D* fgl_glTextureStorage3D;
#		define glTextureStorage3D fgl_glTextureStorage3D
		typedef void (FGL_APIENTRY fgl_func_glTextureStorage2DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		fgl_api fgl_func_glTextureStorage2DMultisample* fgl_glTextureStorage2DMultisample;
#		define glTextureStorage2DMultisample fgl_glTextureStorage2DMultisample
		typedef void (FGL_APIENTRY fgl_func_glTextureStorage3DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		fgl_api fgl_func_glTextureStorage3DMultisample* fgl_glTextureStorage3DMultisample;
#		define glTextureStorage3DMultisample fgl_glTextureStorage3DMultisample
		typedef void (FGL_APIENTRY fgl_func_glTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
		fgl_api fgl_func_glTextureSubImage1D* fgl_glTextureSubImage1D;
#		define glTextureSubImage1D fgl_glTextureSubImage1D
		typedef void (FGL_APIENTRY fgl_func_glTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
		fgl_api fgl_func_glTextureSubImage2D* fgl_glTextureSubImage2D;
#		define glTextureSubImage2D fgl_glTextureSubImage2D
		typedef void (FGL_APIENTRY fgl_func_glTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
		fgl_api fgl_func_glTextureSubImage3D* fgl_glTextureSubImage3D;
#		define glTextureSubImage3D fgl_glTextureSubImage3D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTextureSubImage1D* fgl_glCompressedTextureSubImage1D;
#		define glCompressedTextureSubImage1D fgl_glCompressedTextureSubImage1D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTextureSubImage2D* fgl_glCompressedTextureSubImage2D;
#		define glCompressedTextureSubImage2D fgl_glCompressedTextureSubImage2D
		typedef void (FGL_APIENTRY fgl_func_glCompressedTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
		fgl_api fgl_func_glCompressedTextureSubImage3D* fgl_glCompressedTextureSubImage3D;
#		define glCompressedTextureSubImage3D fgl_glCompressedTextureSubImage3D
		typedef void (FGL_APIENTRY fgl_func_glCopyTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		fgl_api fgl_func_glCopyTextureSubImage1D* fgl_glCopyTextureSubImage1D;
#		define glCopyTextureSubImage1D fgl_glCopyTextureSubImage1D
		typedef void (FGL_APIENTRY fgl_func_glCopyTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glCopyTextureSubImage2D* fgl_glCopyTextureSubImage2D;
#		define glCopyTextureSubImage2D fgl_glCopyTextureSubImage2D
		typedef void (FGL_APIENTRY fgl_func_glCopyTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		fgl_api fgl_func_glCopyTextureSubImage3D* fgl_glCopyTextureSubImage3D;
#		define glCopyTextureSubImage3D fgl_glCopyTextureSubImage3D
		typedef void (FGL_APIENTRY fgl_func_glTextureParameterf)(GLuint texture, GLenum pname, GLfloat param);
		fgl_api fgl_func_glTextureParameterf* fgl_glTextureParameterf;
#		define glTextureParameterf fgl_glTextureParameterf
		typedef void (FGL_APIENTRY fgl_func_glTextureParameterfv)(GLuint texture, GLenum pname, const GLfloat *param);
		fgl_api fgl_func_glTextureParameterfv* fgl_glTextureParameterfv;
#		define glTextureParameterfv fgl_glTextureParameterfv
		typedef void (FGL_APIENTRY fgl_func_glTextureParameteri)(GLuint texture, GLenum pname, GLint param);
		fgl_api fgl_func_glTextureParameteri* fgl_glTextureParameteri;
#		define glTextureParameteri fgl_glTextureParameteri
		typedef void (FGL_APIENTRY fgl_func_glTextureParameterIiv)(GLuint texture, GLenum pname, const GLint *params);
		fgl_api fgl_func_glTextureParameterIiv* fgl_glTextureParameterIiv;
#		define glTextureParameterIiv fgl_glTextureParameterIiv
		typedef void (FGL_APIENTRY fgl_func_glTextureParameterIuiv)(GLuint texture, GLenum pname, const GLuint *params);
		fgl_api fgl_func_glTextureParameterIuiv* fgl_glTextureParameterIuiv;
#		define glTextureParameterIuiv fgl_glTextureParameterIuiv
		typedef void (FGL_APIENTRY fgl_func_glTextureParameteriv)(GLuint texture, GLenum pname, const GLint *param);
		fgl_api fgl_func_glTextureParameteriv* fgl_glTextureParameteriv;
#		define glTextureParameteriv fgl_glTextureParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGenerateTextureMipmap)(GLuint texture);
		fgl_api fgl_func_glGenerateTextureMipmap* fgl_glGenerateTextureMipmap;
#		define glGenerateTextureMipmap fgl_glGenerateTextureMipmap
		typedef void (FGL_APIENTRY fgl_func_glBindTextureUnit)(GLuint unit, GLuint texture);
		fgl_api fgl_func_glBindTextureUnit* fgl_glBindTextureUnit;
#		define glBindTextureUnit fgl_glBindTextureUnit
		typedef void (FGL_APIENTRY fgl_func_glGetTextureImage)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
		fgl_api fgl_func_glGetTextureImage* fgl_glGetTextureImage;
#		define glGetTextureImage fgl_glGetTextureImage
		typedef void (FGL_APIENTRY fgl_func_glGetCompressedTextureImage)(GLuint texture, GLint level, GLsizei bufSize, void *pixels);
		fgl_api fgl_func_glGetCompressedTextureImage* fgl_glGetCompressedTextureImage;
#		define glGetCompressedTextureImage fgl_glGetCompressedTextureImage
		typedef void (FGL_APIENTRY fgl_func_glGetTextureLevelParameterfv)(GLuint texture, GLint level, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetTextureLevelParameterfv* fgl_glGetTextureLevelParameterfv;
#		define glGetTextureLevelParameterfv fgl_glGetTextureLevelParameterfv
		typedef void (FGL_APIENTRY fgl_func_glGetTextureLevelParameteriv)(GLuint texture, GLint level, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTextureLevelParameteriv* fgl_glGetTextureLevelParameteriv;
#		define glGetTextureLevelParameteriv fgl_glGetTextureLevelParameteriv
		typedef void (FGL_APIENTRY fgl_func_glGetTextureParameterfv)(GLuint texture, GLenum pname, GLfloat *params);
		fgl_api fgl_func_glGetTextureParameterfv* fgl_glGetTextureParameterfv;
#		define glGetTextureParameterfv fgl_glGetTextureParameterfv
		typedef void (FGL_APIENTRY fgl_func_glGetTextureParameterIiv)(GLuint texture, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTextureParameterIiv* fgl_glGetTextureParameterIiv;
#		define glGetTextureParameterIiv fgl_glGetTextureParameterIiv
		typedef void (FGL_APIENTRY fgl_func_glGetTextureParameterIuiv)(GLuint texture, GLenum pname, GLuint *params);
		fgl_api fgl_func_glGetTextureParameterIuiv* fgl_glGetTextureParameterIuiv;
#		define glGetTextureParameterIuiv fgl_glGetTextureParameterIuiv
		typedef void (FGL_APIENTRY fgl_func_glGetTextureParameteriv)(GLuint texture, GLenum pname, GLint *params);
		fgl_api fgl_func_glGetTextureParameteriv* fgl_glGetTextureParameteriv;
#		define glGetTextureParameteriv fgl_glGetTextureParameteriv
		typedef void (FGL_APIENTRY fgl_func_glCreateVertexArrays)(GLsizei n, GLuint *arrays);
		fgl_api fgl_func_glCreateVertexArrays* fgl_glCreateVertexArrays;
#		define glCreateVertexArrays fgl_glCreateVertexArrays
		typedef void (FGL_APIENTRY fgl_func_glDisableVertexArrayAttrib)(GLuint vaobj, GLuint index);
		fgl_api fgl_func_glDisableVertexArrayAttrib* fgl_glDisableVertexArrayAttrib;
#		define glDisableVertexArrayAttrib fgl_glDisableVertexArrayAttrib
		typedef void (FGL_APIENTRY fgl_func_glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
		fgl_api fgl_func_glEnableVertexArrayAttrib* fgl_glEnableVertexArrayAttrib;
#		define glEnableVertexArrayAttrib fgl_glEnableVertexArrayAttrib
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
		fgl_api fgl_func_glVertexArrayElementBuffer* fgl_glVertexArrayElementBuffer;
#		define glVertexArrayElementBuffer fgl_glVertexArrayElementBuffer
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayVertexBuffer)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
		fgl_api fgl_func_glVertexArrayVertexBuffer* fgl_glVertexArrayVertexBuffer;
#		define glVertexArrayVertexBuffer fgl_glVertexArrayVertexBuffer
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayVertexBuffers)(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
		fgl_api fgl_func_glVertexArrayVertexBuffers* fgl_glVertexArrayVertexBuffers;
#		define glVertexArrayVertexBuffers fgl_glVertexArrayVertexBuffers
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayAttribBinding)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
		fgl_api fgl_func_glVertexArrayAttribBinding* fgl_glVertexArrayAttribBinding;
#		define glVertexArrayAttribBinding fgl_glVertexArrayAttribBinding
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayAttribFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
		fgl_api fgl_func_glVertexArrayAttribFormat* fgl_glVertexArrayAttribFormat;
#		define glVertexArrayAttribFormat fgl_glVertexArrayAttribFormat
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayAttribIFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		fgl_api fgl_func_glVertexArrayAttribIFormat* fgl_glVertexArrayAttribIFormat;
#		define glVertexArrayAttribIFormat fgl_glVertexArrayAttribIFormat
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayAttribLFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		fgl_api fgl_func_glVertexArrayAttribLFormat* fgl_glVertexArrayAttribLFormat;
#		define glVertexArrayAttribLFormat fgl_glVertexArrayAttribLFormat
		typedef void (FGL_APIENTRY fgl_func_glVertexArrayBindingDivisor)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
		fgl_api fgl_func_glVertexArrayBindingDivisor* fgl_glVertexArrayBindingDivisor;
#		define glVertexArrayBindingDivisor fgl_glVertexArrayBindingDivisor
		typedef void (FGL_APIENTRY fgl_func_glGetVertexArrayiv)(GLuint vaobj, GLenum pname, GLint *param);
		fgl_api fgl_func_glGetVertexArrayiv* fgl_glGetVertexArrayiv;
#		define glGetVertexArrayiv fgl_glGetVertexArrayiv
		typedef void (FGL_APIENTRY fgl_func_glGetVertexArrayIndexediv)(GLuint vaobj, GLuint index, GLenum pname, GLint *param);
		fgl_api fgl_func_glGetVertexArrayIndexediv* fgl_glGetVertexArrayIndexediv;
#		define glGetVertexArrayIndexediv fgl_glGetVertexArrayIndexediv
		typedef void (FGL_APIENTRY fgl_func_glGetVertexArrayIndexed64iv)(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
		fgl_api fgl_func_glGetVertexArrayIndexed64iv* fgl_glGetVertexArrayIndexed64iv;
#		define glGetVertexArrayIndexed64iv fgl_glGetVertexArrayIndexed64iv
		typedef void (FGL_APIENTRY fgl_func_glCreateSamplers)(GLsizei n, GLuint *samplers);
		fgl_api fgl_func_glCreateSamplers* fgl_glCreateSamplers;
#		define glCreateSamplers fgl_glCreateSamplers
		typedef void (FGL_APIENTRY fgl_func_glCreateProgramPipelines)(GLsizei n, GLuint *pipelines);
		fgl_api fgl_func_glCreateProgramPipelines* fgl_glCreateProgramPipelines;
#		define glCreateProgramPipelines fgl_glCreateProgramPipelines
		typedef void (FGL_APIENTRY fgl_func_glCreateQueries)(GLenum target, GLsizei n, GLuint *ids);
		fgl_api fgl_func_glCreateQueries* fgl_glCreateQueries;
#		define glCreateQueries fgl_glCreateQueries
		typedef void (FGL_APIENTRY fgl_func_glGetQueryBufferObjecti64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		fgl_api fgl_func_glGetQueryBufferObjecti64v* fgl_glGetQueryBufferObjecti64v;
#		define glGetQueryBufferObjecti64v fgl_glGetQueryBufferObjecti64v
		typedef void (FGL_APIENTRY fgl_func_glGetQueryBufferObjectiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		fgl_api fgl_func_glGetQueryBufferObjectiv* fgl_glGetQueryBufferObjectiv;
#		define glGetQueryBufferObjectiv fgl_glGetQueryBufferObjectiv
		typedef void (FGL_APIENTRY fgl_func_glGetQueryBufferObjectui64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		fgl_api fgl_func_glGetQueryBufferObjectui64v* fgl_glGetQueryBufferObjectui64v;
#		define glGetQueryBufferObjectui64v fgl_glGetQueryBufferObjectui64v
		typedef void (FGL_APIENTRY fgl_func_glGetQueryBufferObjectuiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		fgl_api fgl_func_glGetQueryBufferObjectuiv* fgl_glGetQueryBufferObjectuiv;
#		define glGetQueryBufferObjectuiv fgl_glGetQueryBufferObjectuiv
		typedef void (FGL_APIENTRY fgl_func_glMemoryBarrierByRegion)(GLbitfield barriers);
		fgl_api fgl_func_glMemoryBarrierByRegion* fgl_glMemoryBarrierByRegion;
#		define glMemoryBarrierByRegion fgl_glMemoryBarrierByRegion
		typedef void (FGL_APIENTRY fgl_func_glGetTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
		fgl_api fgl_func_glGetTextureSubImage* fgl_glGetTextureSubImage;
#		define glGetTextureSubImage fgl_glGetTextureSubImage
		typedef void (FGL_APIENTRY fgl_func_glGetCompressedTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
		fgl_api fgl_func_glGetCompressedTextureSubImage* fgl_glGetCompressedTextureSubImage;
#		define glGetCompressedTextureSubImage fgl_glGetCompressedTextureSubImage
		typedef GLenum (FGL_APIENTRY fgl_func_glGetGraphicsResetStatus)(void);
		fgl_api fgl_func_glGetGraphicsResetStatus* fgl_glGetGraphicsResetStatus;
#		define glGetGraphicsResetStatus fgl_glGetGraphicsResetStatus
		typedef void (FGL_APIENTRY fgl_func_glGetnCompressedTexImage)(GLenum target, GLint lod, GLsizei bufSize, void *pixels);
		fgl_api fgl_func_glGetnCompressedTexImage* fgl_glGetnCompressedTexImage;
#		define glGetnCompressedTexImage fgl_glGetnCompressedTexImage
		typedef void (FGL_APIENTRY fgl_func_glGetnTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
		fgl_api fgl_func_glGetnTexImage* fgl_glGetnTexImage;
#		define glGetnTexImage fgl_glGetnTexImage
		typedef void (FGL_APIENTRY fgl_func_glGetnUniformdv)(GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
		fgl_api fgl_func_glGetnUniformdv* fgl_glGetnUniformdv;
#		define glGetnUniformdv fgl_glGetnUniformdv
		typedef void (FGL_APIENTRY fgl_func_glGetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
		fgl_api fgl_func_glGetnUniformfv* fgl_glGetnUniformfv;
#		define glGetnUniformfv fgl_glGetnUniformfv
		typedef void (FGL_APIENTRY fgl_func_glGetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
		fgl_api fgl_func_glGetnUniformiv* fgl_glGetnUniformiv;
#		define glGetnUniformiv fgl_glGetnUniformiv
		typedef void (FGL_APIENTRY fgl_func_glGetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint *params);
		fgl_api fgl_func_glGetnUniformuiv* fgl_glGetnUniformuiv;
#		define glGetnUniformuiv fgl_glGetnUniformuiv
		typedef void (FGL_APIENTRY fgl_func_glReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
		fgl_api fgl_func_glReadnPixels* fgl_glReadnPixels;
#		define glReadnPixels fgl_glReadnPixels
		typedef void (FGL_APIENTRY fgl_func_glGetnMapdv)(GLenum target, GLenum query, GLsizei bufSize, GLdouble *v);
		fgl_api fgl_func_glGetnMapdv* fgl_glGetnMapdv;
#		define glGetnMapdv fgl_glGetnMapdv
		typedef void (FGL_APIENTRY fgl_func_glGetnMapfv)(GLenum target, GLenum query, GLsizei bufSize, GLfloat *v);
		fgl_api fgl_func_glGetnMapfv* fgl_glGetnMapfv;
#		define glGetnMapfv fgl_glGetnMapfv
		typedef void (FGL_APIENTRY fgl_func_glGetnMapiv)(GLenum target, GLenum query, GLsizei bufSize, GLint *v);
		fgl_api fgl_func_glGetnMapiv* fgl_glGetnMapiv;
#		define glGetnMapiv fgl_glGetnMapiv
		typedef void (FGL_APIENTRY fgl_func_glGetnPixelMapfv)(GLenum map, GLsizei bufSize, GLfloat *values);
		fgl_api fgl_func_glGetnPixelMapfv* fgl_glGetnPixelMapfv;
#		define glGetnPixelMapfv fgl_glGetnPixelMapfv
		typedef void (FGL_APIENTRY fgl_func_glGetnPixelMapuiv)(GLenum map, GLsizei bufSize, GLuint *values);
		fgl_api fgl_func_glGetnPixelMapuiv* fgl_glGetnPixelMapuiv;
#		define glGetnPixelMapuiv fgl_glGetnPixelMapuiv
		typedef void (FGL_APIENTRY fgl_func_glGetnPixelMapusv)(GLenum map, GLsizei bufSize, GLushort *values);
		fgl_api fgl_func_glGetnPixelMapusv* fgl_glGetnPixelMapusv;
#		define glGetnPixelMapusv fgl_glGetnPixelMapusv
		typedef void (FGL_APIENTRY fgl_func_glGetnPolygonStipple)(GLsizei bufSize, GLubyte *pattern);
		fgl_api fgl_func_glGetnPolygonStipple* fgl_glGetnPolygonStipple;
#		define glGetnPolygonStipple fgl_glGetnPolygonStipple
		typedef void (FGL_APIENTRY fgl_func_glGetnColorTable)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table);
		fgl_api fgl_func_glGetnColorTable* fgl_glGetnColorTable;
#		define glGetnColorTable fgl_glGetnColorTable
		typedef void (FGL_APIENTRY fgl_func_glGetnConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image);
		fgl_api fgl_func_glGetnConvolutionFilter* fgl_glGetnConvolutionFilter;
#		define glGetnConvolutionFilter fgl_glGetnConvolutionFilter
		typedef void (FGL_APIENTRY fgl_func_glGetnSeparableFilter)(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span);
		fgl_api fgl_func_glGetnSeparableFilter* fgl_glGetnSeparableFilter;
#		define glGetnSeparableFilter fgl_glGetnSeparableFilter
		typedef void (FGL_APIENTRY fgl_func_glGetnHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
		fgl_api fgl_func_glGetnHistogram* fgl_glGetnHistogram;
#		define glGetnHistogram fgl_glGetnHistogram
		typedef void (FGL_APIENTRY fgl_func_glGetnMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
		fgl_api fgl_func_glGetnMinmax* fgl_glGetnMinmax;
#		define glGetnMinmax fgl_glGetnMinmax
		typedef void (FGL_APIENTRY fgl_func_glTextureBarrier)(void);
		fgl_api fgl_func_glTextureBarrier* fgl_glTextureBarrier;
#		define glTextureBarrier fgl_glTextureBarrier
#	endif // GL_VERSION_4_5

#	ifndef GL_VERSION_4_6
#		define GL_VERSION_4_6 1
		fgl_api bool isGL_VERSION_4_6;

#		define GL_SHADER_BINARY_FORMAT_SPIR_V 0x9551
#		define GL_SPIR_V_BINARY 0x9552
#		define GL_PARAMETER_BUFFER 0x80EE
#		define GL_PARAMETER_BUFFER_BINDING 0x80EF
#		define GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#		define GL_VERTICES_SUBMITTED 0x82EE
#		define GL_PRIMITIVES_SUBMITTED 0x82EF
#		define GL_VERTEX_SHADER_INVOCATIONS 0x82F0
#		define GL_TESS_CONTROL_SHADER_PATCHES 0x82F1
#		define GL_TESS_EVALUATION_SHADER_INVOCATIONS 0x82F2
#		define GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED 0x82F3
#		define GL_FRAGMENT_SHADER_INVOCATIONS 0x82F4
#		define GL_COMPUTE_SHADER_INVOCATIONS 0x82F5
#		define GL_CLIPPING_INPUT_PRIMITIVES 0x82F6
#		define GL_CLIPPING_OUTPUT_PRIMITIVES 0x82F7
#		define GL_POLYGON_OFFSET_CLAMP 0x8E1B
#		define GL_SPIR_V_EXTENSIONS 0x9553
#		define GL_NUM_SPIR_V_EXTENSIONS 0x9554
#		define GL_TEXTURE_MAX_ANISOTROPY 0x84FE
#		define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF
#		define GL_TRANSFORM_FEEDBACK_OVERFLOW 0x82EC
#		define GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW 0x82ED

		typedef void (FGL_APIENTRY fgl_func_glSpecializeShader)(GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue);
		fgl_api fgl_func_glSpecializeShader* fgl_glSpecializeShader;
#		define glSpecializeShader fgl_glSpecializeShader
		typedef void (FGL_APIENTRY fgl_func_glMultiDrawArraysIndirectCount)(GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
		fgl_api fgl_func_glMultiDrawArraysIndirectCount* fgl_glMultiDrawArraysIndirectCount;
#		define glMultiDrawArraysIndirectCount fgl_glMultiDrawArraysIndirectCount
		typedef void (FGL_APIENTRY fgl_func_glMultiDrawElementsIndirectCount)(GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
		fgl_api fgl_func_glMultiDrawElementsIndirectCount* fgl_glMultiDrawElementsIndirectCount;
#		define glMultiDrawElementsIndirectCount fgl_glMultiDrawElementsIndirectCount
		typedef void (FGL_APIENTRY fgl_func_glPolygonOffsetClamp)(GLfloat factor, GLfloat units, GLfloat clamp);
		fgl_api fgl_func_glPolygonOffsetClamp* fgl_glPolygonOffsetClamp;
#		define glPolygonOffsetClamp fgl_glPolygonOffsetClamp
#	endif // GL_VERSION_4_6

#ifdef __cplusplus
}
#endif

#endif // FGL_INCLUDE_H
