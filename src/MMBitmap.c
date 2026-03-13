#include "MMBitmap.h"
#include <assert.h>
#include <string.h>

MMBitmapRef createMMBitmapWithCleanup(uint8_t *buffer,
                                      size_t width,
                                      size_t height,
                                      size_t bytewidth,
                                      uint8_t bitsPerPixel,
                                      uint8_t bytesPerPixel,
                                      void (*bufferDestroy)(char *, void *),
                                      void *bufferDestroyHint)
{
	MMBitmapRef bitmap = malloc(sizeof(MMBitmap));
	if (bitmap == NULL) return NULL;

	bitmap->imageBuffer = buffer;
	bitmap->width = width;
	bitmap->height = height;
	bitmap->bytewidth = bytewidth;
	bitmap->bitsPerPixel = bitsPerPixel;
	bitmap->bytesPerPixel = bytesPerPixel;
	bitmap->bufferDestroy = bufferDestroy;
	bitmap->bufferDestroyHint = bufferDestroyHint;

	return bitmap;
}

MMBitmapRef createMMBitmap(uint8_t *buffer,
                           size_t width,
                           size_t height,
                           size_t bytewidth,
                           uint8_t bitsPerPixel,
                           uint8_t bytesPerPixel)
{
	return createMMBitmapWithCleanup(buffer,
	                                 width,
	                                 height,
	                                 bytewidth,
	                                 bitsPerPixel,
	                                 bytesPerPixel,
	                                 destroyMMBitmapBuffer,
	                                 NULL);
}

void destroyMMBitmap(MMBitmapRef bitmap)
{
	assert(bitmap != NULL);

	if (bitmap->imageBuffer != NULL && bitmap->bufferDestroy != NULL) {
		bitmap->bufferDestroy((char *)bitmap->imageBuffer,
		                      bitmap->bufferDestroyHint);
	}

	if (bitmap->imageBuffer != NULL) {
		bitmap->imageBuffer = NULL;
	}

	free(bitmap);
}

void destroyMMBitmapBuffer(char * bitmapBuffer, void * hint)
{
	if (bitmapBuffer != NULL)	
	{
		free(bitmapBuffer);
	}
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
		size_t y;
		uint8_t *copiedBuf = NULL;
		const size_t rowBytes = rect.size.width * source->bytesPerPixel;
		const size_t bytewidth = ADD_PADDING(rowBytes);
		const size_t bufsize = rect.size.height * bytewidth;

		copiedBuf = malloc(bufsize);
		if (copiedBuf == NULL) return NULL;
		memset(copiedBuf, 0, bufsize);

		for (y = 0; y < rect.size.height; ++y) {
			const size_t sourceOffset = (source->bytewidth * (rect.origin.y + y)) +
			                            (rect.origin.x * source->bytesPerPixel);
			const size_t destOffset = bytewidth * y;

			assert((sourceOffset + rowBytes) <=
			       (source->bytewidth * source->height));
			memcpy(copiedBuf + destOffset,
			       source->imageBuffer + sourceOffset,
			       rowBytes);
		}

		return createMMBitmap(copiedBuf,
		                      rect.size.width,
		                      rect.size.height,
		                      bytewidth,
		                      source->bitsPerPixel,
		                      source->bytesPerPixel);
	}
}
