#include "MMBitmap.h"
#include <assert.h>
#include <string.h>

MMBitmapRef createMMBitmap(uint8_t *buffer,
                           size_t width,
                           size_t height,
                           size_t bytewidth,
                           uint8_t bitsPerPixel,
                           uint8_t bytesPerPixel)
{
	MMBitmapRef bitmap = malloc(sizeof(MMBitmap));
	if (bitmap == NULL) return NULL;

	bitmap->imageBuffer = buffer;
	bitmap->width = width;
	bitmap->height = height;
	bitmap->bytewidth = bytewidth;
	bitmap->bitsPerPixel = bitsPerPixel;
	bitmap->bytesPerPixel = bytesPerPixel;

	return bitmap;
}

void destroyMMBitmap(MMBitmapRef bitmap)
{
	assert(bitmap != NULL);

	if (bitmap->imageBuffer != NULL) {
		free(bitmap->imageBuffer);
		bitmap->imageBuffer = NULL;
	}

	free(bitmap);
}

MMBitmapRef copyMMBitmap(MMBitmapRef bitmap)
{
	uint8_t *copiedBuf = NULL;

	assert(bitmap != NULL);
	if (bitmap->imageBuffer != NULL) {
		const size_t bufsize = bitmap->height * bitmap->bytewidth;
		copiedBuf = malloc(bufsize);
		if (copiedBuf == NULL) return NULL;

		memcpy(copiedBuf, bitmap->imageBuffer, bufsize);
	}

	return createMMBitmap(copiedBuf,
	                      bitmap->width,
	                      bitmap->height,
	                      bitmap->bytewidth,
	                      bitmap->bitsPerPixel,
	                      bitmap->bytesPerPixel);
}

MMBitmapRef copyMMBitmapFromPortion(MMBitmapRef source, MMRect rect)
{
	assert(source != NULL);

	if (source->imageBuffer == NULL || !MMBitmapRectInBounds(source, rect)) {
		return NULL;
	} else {
		uint8_t *copiedBuf = NULL;
		const size_t bufsize = rect.size.height * source->bytewidth;
		const size_t offset = (source->bytewidth * rect.origin.y) +
		                      (rect.origin.x * source->bytesPerPixel);

		/* Don't go over the bounds, programmer! */
		assert((bufsize + offset) <= (source->bytewidth * source->height));

		copiedBuf = malloc(bufsize);
		if (copiedBuf == NULL) return NULL;

		memcpy(copiedBuf, source->imageBuffer + offset, bufsize);

		return createMMBitmap(copiedBuf,
		                      rect.size.width,
		                      rect.size.height,
		                      source->bytewidth,
		                      source->bitsPerPixel,
		                      source->bytesPerPixel);
	}
}
