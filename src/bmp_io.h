#pragma once
#ifndef BMP_IO_H
#define BMP_IO_H

#include "MMBitmap.h"
#include "io.h"

#ifdef __cplusplus
extern "C" 
{
#endif

enum _BMPReadError {
	kBMPGenericError = 0,
	kBMPAccessError,
	kBMPInvalidKeyError,
	kBMPUnsupportedHeaderError,
	kBMPInvalidColorPanesError,
	kBMPUnsupportedColorDepthError,
	kBMPUnsupportedCompressionError,
	kBMPInvalidPixelDataError
};

typedef MMIOError MMBMPReadError;

/* Returns description of given MMBMPReadError.
 * Returned string is constant and hence should not be freed. */
const char *MMBMPReadErrorString(MMIOError error);

/* Attempts to read bitmap file at path; returns new MMBitmap on success, or
 * NULL on error. If |error| is non-NULL, it will be set to the error code
 * on return.
 *
 * Currently supports:
 *     - Uncompressed Windows v3/v4/v5 24-bit or 32-bit BMP.
 *     - OS/2 v1 or v2 24-bit BMP.
 *     - Does NOT yet support: 1-bit, 4-bit, 8-bit, 16-bit, compressed bitmaps,
 *       or PNGs/JPEGs disguised as BMPs (and returns NULL if those are given).
 *
 * Responsibility for destroy()'ing returned MMBitmap is left up to caller. */
MMBitmapRef newMMBitmapFromBMP(const char *path, MMBMPReadError *error);

/* Returns a buffer containing the raw BMP file data in Windows v3 BMP format,
 * ready to be saved to a file. If |len| is not NULL, it will be set to the
 * number of bytes allocated in the returned buffer.
 *
 * Responsibility for free()'ing data is left up to the caller. */
uint8_t *createBitmapData(MMBitmapRef bitmap, size_t *len);

/* Saves bitmap to file in Windows v3 BMP format.
 * Returns 0 on success, -1 on error. */
int saveMMBitmapAsBMP(MMBitmapRef bitmap, const char *path);

/* Swaps bitmap from Quadrant 1 to Quadran III format, or vice versa
 * (upside-down Cartesian/PostScript/GL <-> right side up QD/CG raster format).
 */
void flipBitmapData(void *data, size_t width, size_t height, size_t bytewidth);

#ifdef __cplusplus
}
#endif

#endif /* BMP_IO_H */
