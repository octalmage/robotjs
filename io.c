#include "io.h"
#include "os.h"
#include "bmp_io.h"
#include "png_io.h"
#include <stdio.h> /* For fputs() */
#include <string.h> /* For strcmp() */
#include <ctype.h> /* For tolower() */

const char *getExtension(const char *fname, size_t len)
{
	if (fname == NULL || len <= 0) return NULL;

	while (--len > 0 && fname[len] != '.' && fname[len] != '\0')
		;

	return fname + len + 1;
}

MMImageType imageTypeFromExtension(const char *extension)
{
	char ext[4];
	const size_t maxlen = sizeof(ext) / sizeof(ext[0]);
	size_t i;

	for (i = 0; extension[i] != '\0'; ++i) {
		if (i >= maxlen) return kInvalidImageType;
		ext[i] = tolower(extension[i]);
	}
	ext[i] = '\0';

	if (strcmp(ext, "png") == 0) {
		return kPNGImageType;
	} else if (strcmp(ext, "bmp") == 0) {
		return kBMPImageType;
	} else {
		return kInvalidImageType;
	}
}

MMBitmapRef newMMBitmapFromFile(const char *path, 
                                MMImageType type,
                                MMIOError *err)
{
	switch (type) {
		case kBMPImageType:
			return newMMBitmapFromBMP(path, err);
		case kPNGImageType:
			return newMMBitmapFromPNG(path, err);
		default:
			if (err != NULL) *err = kMMIOUnsupportedTypeError;
			return NULL;
	}
}

int saveMMBitmapToFile(MMBitmapRef bitmap,
                       const char *path,
                       MMImageType type)
{
	switch (type) {
		case kBMPImageType:
			return saveMMBitmapAsBMP(bitmap, path);
		case kPNGImageType:
			return saveMMBitmapAsPNG(bitmap, path);
		default:
			return -1;
	}
}

const char *MMIOErrorString(MMImageType type, MMIOError error)
{
	switch (type) {
		case kBMPImageType:
			return MMBMPReadErrorString(error);
		case kPNGImageType:
			return MMPNGReadErrorString(error);
		default:
			return "Unsupported image type";
	}
}
