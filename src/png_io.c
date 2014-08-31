#include "png_io.h"
#include "os.h"
#include <png.h>
#include <stdio.h> /* fopen() */
#include <stdlib.h> /* malloc/realloc */
#include <assert.h>

#if defined(_MSC_VER)
	#include "ms_stdint.h"
	#include "ms_stdbool.h"
#else
	#include <stdint.h>
	#include <stdbool.h>
#endif

const char *MMPNGReadErrorString(MMIOError error)
{
	switch (error) {
		case kPNGAccessError:
			return "Could not open file";
		case kPNGReadError:
			return "Could not read file";
		case kPNGInvalidHeaderError:
			return "Not a PNG file";
		default:
			return NULL;
	}
}

MMBitmapRef newMMBitmapFromPNG(const char *path, MMPNGReadError *err)
{
	FILE *fp;
	uint8_t header[8];
	png_struct *png_ptr = NULL;
	png_info *info_ptr = NULL;
	png_byte bit_depth, color_type;
	uint8_t *row, *bitmapData;
	uint8_t bytesPerPixel;
	png_uint_32 width, height, y;
	uint32_t bytewidth;

	if ((fp = fopen(path, "rb")) == NULL) {
		if (err != NULL) *err = kPNGAccessError;
		return NULL;
	}

	/* Initialize error code to generic value. */
	if (err != NULL) *err = kPNGGenericError;

	/* Validate the PNG. */
	if (fread(header, 1, sizeof header, fp) == 0) {
		if (err != NULL) *err = kPNGReadError;
		goto bail;
	} else if (png_sig_cmp(header, 0, sizeof(header)) != 0) {
		if (err != NULL) *err = kPNGInvalidHeaderError;
		goto bail;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) goto bail;

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) goto bail;

	/* Set up error handling. */
	if (setjmp(png_jmpbuf(png_ptr))) {
		goto bail;
	}

	png_init_io(png_ptr, fp);

	/* Skip past the header. */
	png_set_sig_bytes(png_ptr, sizeof header);

	png_read_info(png_ptr, info_ptr);

	/* Convert different image types to common type to be read. */
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Convert color palettes to RGB. */
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	/* Convert PNG to bit depth of 8. */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	} else if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	/* Convert transparency chunk to alpha channel. */
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))  {
		png_set_tRNS_to_alpha(png_ptr);
	}

	/* Convert gray images to RGB. */
	if (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	/* Ignore alpha for now. */
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		png_set_strip_alpha(png_ptr);
	}

	/* Get image attributes. */
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	bytesPerPixel = 3; /* All images decompress to this size. */
	bytewidth = ADD_PADDING(width * bytesPerPixel); /* Align width. */

	/* Decompress the PNG row by row. */
	bitmapData = calloc(1, bytewidth * height);
	row = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
	if (bitmapData == NULL || row == NULL) goto bail;
	for (y = 0; y < height; ++y) {
		png_uint_32 x;
		const uint32_t rowOffset = y * bytewidth;
		uint8_t *rowptr = row;
		png_read_row(png_ptr, (png_byte *)row, NULL);

		for (x = 0; x < width; ++x) {
			const uint32_t colOffset = x * bytesPerPixel;
			MMRGBColor *color = (MMRGBColor *)(bitmapData + rowOffset + colOffset);
			color->red = *rowptr++;
			color->green = *rowptr++;
			color->blue = *rowptr++;
		}
	}
	free(row);

	/* Finish reading. */
	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);

	return createMMBitmap(bitmapData, width, height,
	                      bytewidth, bytesPerPixel * 8, bytesPerPixel);

bail:
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return NULL;
}

struct _PNGWriteInfo {
	png_struct *png_ptr;
	png_info *info_ptr;
	png_byte **row_pointers;
	size_t row_count;
	bool free_row_pointers;
};

typedef struct _PNGWriteInfo PNGWriteInfo;
typedef PNGWriteInfo *PNGWriteInfoRef;

/* Returns pointer to PNGWriteInfo struct containing data ready to be used with
 * functions such as png_write_png().
 *
 * It is the caller's responsibility to destroy() the returned structure with
 * destroyPNGWriteInfo(). */
static PNGWriteInfoRef createPNGWriteInfo(MMBitmapRef bitmap)
{
	PNGWriteInfoRef info = malloc(sizeof(PNGWriteInfo));
	png_uint_32 y;

	if (info == NULL) return NULL;
	info->png_ptr = NULL;
	info->info_ptr = NULL;
	info->row_pointers = NULL;

	assert(bitmap != NULL);

	/* Initialize the write struct. */
	info->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	                                        NULL, NULL, NULL);
	if (info->png_ptr == NULL) goto bail;

	/* Set up error handling. */
	if (setjmp(png_jmpbuf(info->png_ptr))) {
		png_destroy_write_struct(&(info->png_ptr), &(info->info_ptr));
		goto bail;
	}

	/* Initialize the info struct. */
	info->info_ptr = png_create_info_struct(info->png_ptr);
	if (info->info_ptr == NULL) {
		png_destroy_write_struct(&(info->png_ptr), NULL);
		goto bail;
	}

	/* Set image attributes. */
	png_set_IHDR(info->png_ptr,
	             info->info_ptr,
	             (png_uint_32)bitmap->width,
	             (png_uint_32)bitmap->height,
	             8,
	             PNG_COLOR_TYPE_RGB,
	             PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT,
	             PNG_FILTER_TYPE_DEFAULT);

	info->row_count = bitmap->height;
	info->row_pointers = png_malloc(info->png_ptr,
	                                sizeof(png_byte *) * info->row_count);

	if (bitmap->bytesPerPixel == 3) {
		/* No alpha channel; image data can be copied directly. */
		for (y = 0; y < info->row_count; ++y) {
			info->row_pointers[y] = bitmap->imageBuffer + (bitmap->bytewidth * y);
		}
		info->free_row_pointers = false;

		/* Convert BGR to RGB if necessary. */
		if (MMRGB_IS_BGR) {
			png_set_bgr(info->png_ptr);
		}
	} else {
		/* Ignore alpha channel; copy image data row by row. */
		const size_t bytesPerPixel = 3;
		const size_t bytewidth = ADD_PADDING(bitmap->width * bytesPerPixel);

		for (y = 0; y < info->row_count; ++y) {
			png_uint_32 x;
			png_byte *row_ptr = png_malloc(info->png_ptr, bytewidth);
			info->row_pointers[y] = row_ptr;
			for (x = 0; x < bitmap->width; ++x) {
				MMRGBColor *color = MMRGBColorRefAtPoint(bitmap, x, y);
				row_ptr[0] = color->red;
				row_ptr[1] = color->green;
				row_ptr[2] = color->blue;

				row_ptr += bytesPerPixel;
			}
		}
		info->free_row_pointers = true;
	}

	png_set_rows(info->png_ptr, info->info_ptr, info->row_pointers);
	return info;

bail:
	if (info != NULL) free(info);
	return NULL;
}

/* Free memory in use by |info|. */
static void destroyPNGWriteInfo(PNGWriteInfoRef info)
{
	assert(info != NULL);
	if (info->row_pointers != NULL) {
		if (info->free_row_pointers) {
			size_t y;
			for (y = 0; y < info->row_count; ++y) {
				free(info->row_pointers[y]);
			}
		}
		png_free(info->png_ptr, info->row_pointers);
	}

	png_destroy_write_struct(&(info->png_ptr), &(info->info_ptr));
	free(info);
}

int saveMMBitmapAsPNG(MMBitmapRef bitmap, const char *path)
{
	FILE *fp = fopen(path, "wb");
	PNGWriteInfoRef info;
	if (fp == NULL) return -1;

	if ((info = createPNGWriteInfo(bitmap)) == NULL) {
		fclose(fp);
		return -1;
	}

	png_init_io(info->png_ptr, fp);
	png_write_png(info->png_ptr, info->info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	fclose(fp);

	destroyPNGWriteInfo(info);
	return 0;
}

/* Structure to store PNG image bytes. */
struct io_data
{
	uint8_t *buffer; /* Pointer to raw file data. */
	size_t size; /* Number of bytes actually written to buffer. */
	size_t allocedSize; /* Number of bytes allocated for buffer. */
};

/* Called each time libpng attempts to write data in createPNGData(). */
void png_append_data(png_struct *png_ptr,
                     png_byte *new_data,
                     png_size_t length)
{
	struct io_data *data = png_get_io_ptr(png_ptr);
	data->size += length;

	/* Allocate or grow buffer. */
	if (data->buffer == NULL) {
		data->allocedSize = data->size;
		data->buffer = png_malloc(png_ptr, data->allocedSize);
		assert(data->buffer != NULL);
	} else if (data->allocedSize < data->size) {
		do {
			/* Double size each time to avoid calls to realloc. */
			data->allocedSize <<= 1;
		} while (data->allocedSize < data->size);

		data->buffer = realloc(data->buffer, data->allocedSize);
	}

	/* Copy new bytes to end of buffer. */
	memcpy(data->buffer + data->size - length, new_data, length);
}

uint8_t *createPNGData(MMBitmapRef bitmap, size_t *len)
{
	PNGWriteInfoRef info = NULL;
	struct io_data data = {NULL, 0, 0};

	assert(bitmap != NULL);
	assert(len != NULL);

	if ((info = createPNGWriteInfo(bitmap)) == NULL) return NULL;

	png_set_write_fn(info->png_ptr, &data, &png_append_data, NULL);
	png_write_png(info->png_ptr, info->info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	destroyPNGWriteInfo(info);

	*len = data.size;
	return data.buffer;
}
