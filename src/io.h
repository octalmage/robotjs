#pragma once
#ifndef IO_H
#define IO_H

#include "MMBitmap.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif

enum _MMImageType {
	kInvalidImageType = 0,
	kPNGImageType,
	kBMPImageType /* Currently only PNG and BMP are supported. */
};

typedef uint16_t MMImageType;

enum _MMIOError {
	kMMIOUnsupportedTypeError = 0
};

typedef uint16_t MMIOError;

const char *getExtension(const char *fname, size_t len);

/* Returns best guess at the MMImageType based on a file extension, or
 * |kInvalidImageType| if no matching type was found. */
MMImageType imageTypeFromExtension(const char *ext);

/* Attempts to parse the file of the given type at the given path.
 * |filepath| is an ASCII string describing the absolute POSIX path.
 * Returns new bitmap (to be destroy()'d by caller) on success, NULL on error.
 * If |error| is non-NULL, it will be set to the error code on return.
 */
MMBitmapRef newMMBitmapFromFile(const char *path, MMImageType type, MMIOError *err);

/* Saves |bitmap| to a file of the given type at the given path.
 * |filepath| is an ASCII string describing the absolute POSIX path.
 * Returns 0 on success, -1 on error. */
int saveMMBitmapToFile(MMBitmapRef bitmap, const char *path, MMImageType type);

/* Returns description of given error code.
 * Returned string is constant and hence should not be freed. */
const char *MMIOErrorString(MMImageType type, MMIOError error);

#ifdef __cplusplus
}
#endif


#endif /* IO_H */
