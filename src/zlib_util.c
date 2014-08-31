#include "zlib_util.h"
#include <zlib.h>
#include <stdio.h> /* fprintf() */
#include <stdlib.h> /* malloc() */
#include <assert.h>

#define ZLIB_CHUNK (16 * 1024)

uint8_t *zlib_decompress(const uint8_t *buf, size_t *len)
{
	size_t output_size = ZLIB_CHUNK;
	uint8_t *output = malloc(output_size);
	int err;
	z_stream zst;

	/* Sanity check */
	if (output == NULL) return NULL;
	assert(buf != NULL);

	/* Set inflate state */
	zst.zalloc = Z_NULL;
	zst.zfree = Z_NULL;
	zst.opaque = Z_NULL;
	zst.next_out = (Byte *)output;
	zst.next_in = (Byte *)buf;
	zst.avail_out = ZLIB_CHUNK;

	if (inflateInit(&zst) != Z_OK) goto error;

	/* Decompress input buffer */
	do {
		if ((err = inflate(&zst, Z_NO_FLUSH)) == Z_OK) { /* Need more memory */
			zst.avail_out = (uInt)output_size;

			/* Double size each time to avoid calls to realloc() */
			output_size <<= 1;
			output = realloc(output, output_size + 1);
			if (output == NULL) return NULL;

			zst.next_out = (Byte *)(output + zst.avail_out);
		} else if (err != Z_STREAM_END) { /* Error decompressing */
			if (zst.msg != NULL) {
				fprintf(stderr, "Could not decompress data: %s\n", zst.msg);
			}
			inflateEnd(&zst);
			goto error;
		}
	} while (err != Z_STREAM_END);

	if (len != NULL) *len = zst.total_out;
	if (inflateEnd(&zst) != Z_OK) goto error;
	return output; /* To be free()'d by caller */

error:
	if (output != NULL) free(output);
	return NULL;
}

uint8_t *zlib_compress(const uint8_t *buf, const size_t buflen, int level,
                       size_t *len)
{
	z_stream zst;
	uint8_t *output = NULL;

	/* Sanity check */
	assert(buf != NULL);
	assert(len != NULL);
	assert(level <= 9 && level >= 0);

	zst.avail_out = (uInt)((buflen + (buflen / 10)) + 12);
	output = malloc(zst.avail_out);
	if (output == NULL) return NULL;

	/* Set deflate state */
	zst.zalloc = Z_NULL;
	zst.zfree = Z_NULL;
	zst.next_out = (Byte *)output;
	zst.next_in = (Byte *)buf;
	zst.avail_in = (uInt)buflen;

	if (deflateInit(&zst, level) != Z_OK) goto error;

	/* Compress input buffer */
	if (deflate(&zst, Z_FINISH) != Z_STREAM_END) {
		if (zst.msg != NULL) {
			fprintf(stderr, "Could not compress data: %s\n", zst.msg);
		}
		deflateEnd(&zst);
		goto error;
	}

	if (len != NULL) *len = zst.total_out;
	if (deflateEnd(&zst) != Z_OK) goto error;
	return output; /* To be free()'d by caller */

error:
	if (output != NULL) free(output);
	return NULL;
}
