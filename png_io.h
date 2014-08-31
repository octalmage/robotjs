#pragma once
#ifndef PNG_IO_H
#define PNG_IO_H

#include "MMBitmap.h"
#include "io.h"

enum _PNGReadError {
	kPNGGenericError = 0,
	kPNGReadError,
	kPNGAccessError,
	kPNGInvalidHeaderError
};

typedef MMIOError MMPNGReadError;

/* Returns description of given MMPNGReadError.
 * Returned string is constant and hence should not be freed. */
const char *MMPNGReadErrorString(MMIOError error);

/* Attempts to read PNG file at path; returns new MMBitmap on success, or
 * NULL on error. If |error| is non-NULL, it will be set to the error code
 * on return.
 * Responsibility for destroy()'ing returned MMBitmap is left up to caller. */
MMBitmapRef newMMBitmapFromPNG(const char *path, MMPNGReadError *error);

/* Attempts to write PNG at path; returns 0 on success, -1 on error. */
int saveMMBitmapAsPNG(MMBitmapRef bitmap, const char *path);

/* Returns a buffer containing the raw PNG file data, ready to be saved to a
 * file. |len| will be set to the number of bytes allocated in the returned
 * buffer (it cannot be NULL).
 *
 * Responsibility for free()'ing data is left up to the caller. */
uint8_t *createPNGData(MMBitmapRef bitmap, size_t *len);

#endif /* PNG_IO_H */
