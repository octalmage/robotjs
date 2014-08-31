#include "str_io.h"
#include "zlib_util.h"
#include "base64.h"
#include "snprintf.h" /* snprintf() */
#include <stdio.h> /* fputs() */
#include <ctype.h> /* isdigit() */
#include <stdlib.h> /* atoi() */
#include <string.h> /* strlen() */
#include <assert.h>

#if defined(_MSC_VER)
	#include "ms_stdbool.h"
#else
	#include <stdbool.h>
#endif

#define STR_BITS_PER_PIXEL 24
#define STR_BYTES_PER_PIXEL ((STR_BITS_PER_PIXEL) / 8)

#define MAX_DIMENSION_LEN 5 /* Maximum length for [width] or [height]
                             * in string. */

const char *MMBitmapStringErrorString(MMBMPStringError err)
{
	switch (err) {
		case kMMBMPStringInvalidHeaderError:
			return "Invalid header for string";
		case kMMBMPStringDecodeError:
			return "Error decoding string";
		case kMMBMPStringDecompressError:
			return "Error decompressing string";
		case kMMBMPStringSizeError:
			return "String not of expected size";
		case MMMBMPStringEncodeError:
			return "Error encoding string";
		case kMMBMPStringCompressError:
			return "Error compressing string";
		default:
			return NULL;
	}
}

/* Parses beginning of string in the form of "[width],[height],*".
 *
 * If successful, |width| and |height| are set to the appropropriate values,
 * |len| is set to the length of [width] + the length of [height] + 2,
 * and true is returned; otherwise, false is returned.
 */
static bool getSizeFromString(const uint8_t *buf, size_t buflen,
                              size_t *width, size_t *height,
                              size_t *len);

MMBitmapRef createMMBitmapFromString(const uint8_t *buffer, size_t buflen,
                                     MMBMPStringError *err)
{
	uint8_t *decoded, *decompressed;
	size_t width, height;
	size_t len, bytewidth;

	if (*buffer++ != 'b' || !getSizeFromString(buffer, --buflen,
	                                           &width, &height, &len)) {
		if (err != NULL) *err = kMMBMPStringInvalidHeaderError;
		return NULL;
	}
	buffer += len;
	buflen -= len;

	decoded = base64decode(buffer, buflen, NULL);
	if (decoded == NULL) {
		if (err != NULL) *err = kMMBMPStringDecodeError;
		return NULL;
	}

	decompressed = zlib_decompress(decoded, &len);
	free(decoded);

	if (decompressed == NULL) {
		if (err != NULL) *err = kMMBMPStringDecompressError;
		return NULL;
	}

	bytewidth = width * STR_BYTES_PER_PIXEL; /* Note that bytewidth is NOT
	                                          * aligned to a padding. */
	if (height * bytewidth != len) {
		if (err != NULL) *err = kMMBMPStringSizeError;
		return NULL;
	}

	return createMMBitmap(decompressed, width, height,
	                      bytewidth, STR_BITS_PER_PIXEL, STR_BYTES_PER_PIXEL);
}

/* Returns bitmap data suitable for encoding to a string; that is, 24-bit BGR
 * bitmap with no padding and 3 bytes per pixel.
 *
 * Caller is responsible for free()'ing returned buffer. */
static uint8_t *createRawBitmapData(MMBitmapRef bitmap);

uint8_t *createStringFromMMBitmap(MMBitmapRef bitmap, MMBMPStringError *err)
{
	uint8_t *raw, *compressed;
	uint8_t *ret, *encoded;
	size_t len, retlen;

	assert(bitmap != NULL);

	raw = createRawBitmapData(bitmap);
	if (raw == NULL) {
		if (err != NULL) *err = kMMBMPStringGenericError;
		return NULL;
	}

	compressed = zlib_compress(raw,
	                           bitmap->width * bitmap->height *
	                           STR_BYTES_PER_PIXEL,
	                           9, &len);
	free(raw);
	if (compressed == NULL) {
		if (err != NULL) *err = kMMBMPStringCompressError;
		return NULL;
	}

	encoded = base64encode(compressed, len - 1, &retlen);
	free(compressed);
	if (encoded == NULL) {
		if (err != NULL) *err = MMMBMPStringEncodeError;
		return NULL;
	}

	retlen += 3 + (MAX_DIMENSION_LEN * 2);
	ret = calloc(sizeof(char), (retlen + 1));
	snprintf((char *)ret, retlen, "b%lu,%lu,%s", (unsigned long)bitmap->width,
	                                             (unsigned long)bitmap->height,
												 encoded);
	ret[retlen] = '\0';
	free(encoded);
	return ret;
}

static uint32_t parseDimension(const uint8_t *buf, size_t buflen,
                               size_t *numlen);

static bool getSizeFromString(const uint8_t *buf, size_t buflen,
                              size_t *width, size_t *height,
                              size_t *len)
{
	size_t numlen;
	assert(buf != NULL);
	assert(width != NULL);
	assert(height != NULL);

	if ((*width = parseDimension(buf, buflen, &numlen)) == 0) {
		return false;
	}
	*len = numlen + 1;

	if ((*height = parseDimension(buf + *len, buflen, &numlen)) == 0) {
		return false;
	}
	*len += numlen + 1;

	return true;
}

/* Parses one dimension from string as described in getSizeFromString().
 * Returns dimension on success, or 0 on error. */
static uint32_t parseDimension(const uint8_t *buf, size_t buflen,
                               size_t *numlen)
{
	char num[MAX_DIMENSION_LEN + 1];
	size_t i;

	assert(buf != NULL);
	assert(len != NULL);
	for (i = 0; i < buflen && buf[i] != ',' && buf[i] != '\0'; ++i) {
		if (!isdigit(buf[i]) || i > MAX_DIMENSION_LEN) return 0;
		num[i] = buf[i];
	}
	num[i] = '\0';
	*numlen = i;

	return (uint32_t)atoi(num);
}

static uint8_t *createRawBitmapData(MMBitmapRef bitmap)
{
	uint8_t *raw = calloc(STR_BYTES_PER_PIXEL, bitmap->width * bitmap->height);
	size_t y;

	for (y = 0; y < bitmap->height; ++y) {
		/* No padding is added to string bitmaps. */
		const size_t rowOffset = y * bitmap->width * STR_BYTES_PER_PIXEL;
		size_t x;
		for (x = 0; x < bitmap->width; ++x) {
			/* Copy in BGR format. */
			const size_t colOffset = x * STR_BYTES_PER_PIXEL;
			uint8_t *dest = raw + rowOffset + colOffset;
			MMRGBColor *srcColor = MMRGBColorRefAtPoint(bitmap, x, y);
			dest[0] = srcColor->blue;
			dest[1] = srcColor->green;
			dest[2] = srcColor->red;
		}
	}

	return raw;
}
