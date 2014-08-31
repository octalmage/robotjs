#pragma once
#ifndef PASTEBOARD_H
#define PASTEBOARD_H

#include "MMBitmap.h"
#include "io.h"

enum _MMBitmapPasteError {
	kMMPasteNoError = 0,
	kMMPasteGenericError,
	kMMPasteOpenError,
	kMMPasteClearError,
	kMMPasteDataError,
	kMMPastePasteError,
	kMMPasteUnsupportedError
};

typedef MMIOError MMPasteError;

/* Copies |bitmap| to the pasteboard as a PNG.
 * Returns 0 on success, non-zero on error. */
MMPasteError copyMMBitmapToPasteboard(MMBitmapRef bitmap);

/* Returns description of given MMPasteError.
 * Returned string is constant and hence should not be freed. */
const char *MMPasteErrorString(MMPasteError error);

#endif /* PASTEBOARD_H */
