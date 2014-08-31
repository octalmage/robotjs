#pragma once
#ifndef ENDIAN_H
#define ENDIAN_H

#include "os.h"

/*
 * (Mostly) cross-platform endian definitions and bit swapping macros.
 * Unfortunately, there is no standard C header for this, so we just
 * include the most common ones and fallback to our own custom macros.
 */

#if defined(__linux__) /* Linux */
	#include <endian.h>
	#include <byteswap.h>
#elif (defined(__FreeBSD__) && __FreeBSD_version >= 470000) || \
       defined(__OpenBSD__) || defined(__NetBSD__) /* (Free|Open|Net)BSD */
	#include <sys/endian.h>
	#define __BIG_ENDIAN BIG_ENDIAN
	#define __LITTLE_ENDIAN LITTLE_ENDIAN
	#define __BYTE_ORDER BYTE_ORDER
#elif defined(IS_MACOSX) || (defined(BSD) && (BSD >= 199103)) /* Other BSD */
	#include <machine/endian.h>
	#define __BIG_ENDIAN BIG_ENDIAN
	#define __LITTLE_ENDIAN LITTLE_ENDIAN
	#define __BYTE_ORDER BYTE_ORDER
#elif defined(IS_WINDOWS) /* Windows is assumed to be little endian only. */
	#define __BIG_ENDIAN 4321
	#define __LITTLE_ENDIAN 1234
	#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

/* Fallback to custom constants. */
#if !defined(__BIG_ENDIAN)
	#define __BIG_ENDIAN 4321
#endif

#if !defined(__LITTLE_ENDIAN)
	#define __LITTLE_ENDIAN 1234
#endif

/* Prefer compiler flag settings if given. */
#if defined(MM_BIG_ENDIAN)
	#undef __BYTE_ORDER /* Avoid redefined macro compiler warning. */
	#define __BYTE_ORDER  __BIG_ENDIAN
#elif defined(MM_LITTLE_ENDIAN)
	#undef __BYTE_ORDER /* Avoid redefined macro compiler warning. */
	#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

/* Define default endian-ness. */
#ifndef __LITTLE_ENDIAN
	#define __LITTLE_ENDIAN 1234
#endif /* __LITTLE_ENDIAN */

#ifndef __BIG_ENDIAN
	#define __BIG_ENDIAN 4321
#endif /* __BIG_ENDIAN */

#ifndef __BYTE_ORDER
	#warning "Byte order not defined on your system; assuming little endian"
	#define __BYTE_ORDER __LITTLE_ENDIAN
#endif /* __BYTE_ORDER */

#if __BYTE_ORDER != __BIG_ENDIAN && __BYTE_ORDER != __LITTLE_ENDIAN
	#error "__BYTE_ORDER set to unknown byte order"
#endif

#if defined(IS_MACOSX)
	#include <libkern/OSByteOrder.h>

	/* OS X system functions. */
	#define bitswap16(i) OSSwapInt16(i)
	#define bitswap32(i) OSSwapInt32(i)
	#define swapLittleAndHost32(i) OSSwapLittleToHostInt32(i)
	#define swapLittleAndHost16(i) OSSwapLittleToHostInt16(i)
#else
	#ifndef bitswap16
		#if defined(bswap16)
			#define bitswap16(i) bswap16(i) /* FreeBSD system function */
		#elif defined(bswap_16)
			#define bitswap16(i) bswap_16(i) /* Linux system function */
		#else /* Default macro */
			#define bitswap16(i) (((uint16_t)(i) & 0xFF00) >> 8) | \
 		                          (((uint16_t)(i) & 0x00FF) << 8)
		#endif
	#endif /* bitswap16 */

	#ifndef bitswap32
		#if defined(bswap32)
			#define bitswap32(i) bswap32(i) /* FreeBSD system function. */
		#elif defined(bswap_32)
			#define bitswap32(i) bswap_32(i) /* Linux system function. */
		#else /* Default macro */
			#define bitswap32(i) (((uint32_t)(i) & 0xFF000000) >> 24) | \
			                      ((uint32_t)((i) & 0x00FF0000) >> 8) | \
			                      ((uint32_t)((i) & 0x0000FF00) << 8) | \
			                      ((uint32_t)((i) & 0x000000FF) << 24)
		#endif
	#endif /* bitswap32 */
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
	/* Little endian to/from host byte order (big endian). */
	#ifndef swapLittleAndHost16
		#define swapLittleAndHost16(i) bitswap16(i)
	#endif /* swapLittleAndHost16 */

	#ifndef swapLittleAndHost32
		#define swapLittleAndHost32(i) bitswap32(i)
	#endif /* swapLittleAndHost32 */
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	/* We are already little endian, so no conversion is needed. */
	#ifndef swapLittleAndHost16
		#define swapLittleAndHost16(i) i
	#endif /* swapLittleAndHost16 */

	#ifndef swapLittleAndHost32
		#define swapLittleAndHost32(i) i
	#endif /* swapLittleAndHost32 */
#endif

#endif /* ENDIAN_H */
