#pragma once
#ifndef ZLIB_UTIL_H
#define ZLIB_UTIL_H

#include <stddef.h>

#if defined(_MSC_VER)
	#include "ms_stdint.h"
#else
	#include <stdint.h>
#endif

/* Attempts to decompress given deflated NUL-terminated buffer.
 *
 * If successful and |len| is not NULL, |len| will be set to the number of
 * bytes in the returned buffer.
 * Returns new string to be free()'d by caller, or NULL on error. */
uint8_t *zlib_decompress(const uint8_t *buf, size_t *len);

/* Attempt to compress given buffer.
 *
 * The compression level is passed directly to zlib: it must between 0 and 9,
 * where 1 gives best speed, 9 gives best compression, and 0 gives no
 * compression at all.
 *
 * If successful and |len| is not NULL, |len| will be set to the number of
 * bytes in the returned buffer.
 * Returns new string to be free()'d by caller, or NULL on error. */
uint8_t *zlib_compress(const uint8_t *buf, const size_t buflen, int level,
                       size_t *len);

#endif /* ZLIB_UTIL_H */
