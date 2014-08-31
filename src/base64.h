#pragma once
#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

#if defined(_MSC_VER)
	#include "ms_stdint.h"
#else
	#include <stdint.h>
#endif

/* Decode a base64 encoded string discarding line breaks and noise.
 *
 * Returns a new string to be free()'d by caller, or NULL on error.
 * Returned string is guaranteed to be NUL-terminated.
 *
 * If |retlen| is not NULL, it is set to the length of the returned string
 * (minus the NUL-terminator) on successful return. */
uint8_t *base64decode(const uint8_t *buf, const size_t buflen, size_t *retlen);

/* Encode a base64 encoded string without line breaks or noise.
 *
 * Returns a new string to be free()'d by caller, or NULL on error.
 * Returned string is guaranteed to be NUL-terminated with the correct padding.
 *
 * If |retlen| is not NULL, it is set to the length of the returned string
 * (minus the NUL-terminator) on successful return. */
uint8_t *base64encode(const uint8_t *buf, const size_t buflen, size_t *retlen);

#endif /* BASE64_H */
