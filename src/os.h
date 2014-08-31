#pragma once
#ifndef OS_H
#define OS_H

/* Python versions under 2.5 don't support this macro, but it's not
 * terribly difficult to replicate: */
#ifndef PyModule_AddIntMacro
	#define PyModule_AddIntMacro(module, macro) \
		PyModule_AddIntConstant(module, #macro, macro)
#endif /* PyModule_AddIntMacro */

#if !defined(IS_MACOSX) && defined(__APPLE__) && defined(__MACH__)
	#define IS_MACOSX
#endif /* IS_MACOSX */

#if !defined(IS_WINDOWS) && (defined(WIN32) || defined(_WIN32) || \
                             defined(__WIN32__) || defined(__WINDOWS__))
	#define IS_WINDOWS
#endif /* IS_WINDOWS */

#if !defined(USE_X11) && !defined(NUSE_X11) && !defined(IS_MACOSX) && !defined(IS_WINDOWS)
	#define USE_X11
#endif /* USE_X11 */

#if defined(IS_WINDOWS)
	#define STRICT /* Require use of exact types. */
	#define WIN32_LEAN_AND_MEAN 1 /* Speed up compilation. */
	#include <windows.h>
#elif !defined(IS_MACOSX) && !defined(USE_X11)
	#error "Sorry, this platform isn't supported yet!"
#endif

/* Interval to align by for large buffers (e.g. bitmaps). */
/* Must be a power of 2. */
#ifndef BYTE_ALIGN
	#define BYTE_ALIGN 4 /* Bytes to align pixel buffers to. */
	/* #include <stddef.h> */
	/* #define BYTE_ALIGN (sizeof(size_t)) */
#endif /* BYTE_ALIGN */

#if BYTE_ALIGN == 0
	/* No alignment needed. */
	#define ADD_PADDING(width) (width)
#else
	/* Aligns given width to padding. */
	#define ADD_PADDING(width) (BYTE_ALIGN + (((width) - 1) & ~(BYTE_ALIGN - 1)))
#endif

#endif /* OS_H */
