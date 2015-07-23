#pragma once
#ifndef STR_IO_H
#define STR_IO_H

#include "MMBitmap.h"
#include "io.h"
#include <stdint.h>


enum _MMBMPStringError {
	kMMBMPStringGenericError = 0,
	kMMBMPStringInvalidHeaderError,
	kMMBMPStringDecodeError,
	kMMBMPStringDecompressError,
	kMMBMPStringSizeError, /* Size does not match header. */
	MMMBMPStringEncodeError,
	kMMBMPStringCompressError
};

typedef MMIOError MMBMPStringError;

/* Creates a 24-bit bitmap from a compressed, printable string.
 *
 * String should be in the format: "b[width],[height],[data]",
 * where [width] and [height] are the image width & height, and [data]
 * is the raw image data run through zlib_compress() and base64_encode().
 *
 * Returns NULL on error; follows the Create Rule (that is, the caller is
 * responsible for destroy'()ing object).
 * If |error| is non-NULL, it will be set to the error code on return.
 */
MMBitmapRef createMMBitmapFromString(const uint8_t *buffer, size_t buflen,
                                     MMBMPStringError *error);

/* Inverse of createMMBitmapFromString().
 *
 * Creates string in the format: "b[width],[height],[data]", where [width] and
 * [height] are the image width & height, and [data] is the raw image data run
 * through zlib_compress() and base64_encode().
 *
 * Returns NULL on error, or new string on success (to be free'()d by caller).
 * If |error| is non-NULL, it will be set to the error code on return.
 */
uint8_t *createStringFromMMBitmap(MMBitmapRef bitmap, MMBMPStringError *error);

/* Returns description of given error code.
 * Returned string is constant and hence should not be freed. */
const char *MMBitmapStringErrorString(MMBMPStringError err);

#endif /* STR_IO_H */
