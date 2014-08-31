#include "bmp_io.h"
#include "os.h"
#include "endian.h"
#include <stdio.h> /* fopen() */
#include <string.h> /* memcpy() */

#if defined(_MSC_VER)
	#include "ms_stdbool.h"
	#include "ms_stdint.h"
#else
	#include <stdbool.h>
	#include <stdint.h>
#endif

#pragma pack(push, 1) /* The following structs should be continguous, so we can
                       * copy them in one read. */
/*
 * Standard, initial BMP Header
 */
struct BITMAP_FILE_HEADER {
	uint16_t magic;       /* First two byes of the file; should be 0x4D42. */
	uint32_t fileSize;    /* Size of the BMP file in bytes (unreliable). */
	uint32_t reserved;    /* Application-specific. */
	uint32_t imageOffset; /* Offset to bitmap data. */
};

#define BMP_MAGIC 0x4D42 /* The starting key that marks the file as a BMP. */

enum _BMP_COMPRESSION {
	kBMP_RGB = 0, /* No compression. */
	kBMP_RLE8 = 1, /* Can only be used with 8-bit bitmaps. */
	kBMP_RLE4 = 2, /* Can only be used with 4-bit bitmaps. */
	kBMP_BITFIELDS = 3, /* Can only be used with 16/32-bit bitmaps. */
	kBMP_JPEG = 4, /* Bitmap contains a JPEG image. */
	kBMP_PNG = 5 /* Bitmap contains a PNG image. */
};

typedef uint32_t BMP_COMPRESSION;

/*
 * Windows 3 Header
 */
struct BITMAP_INFO_HEADER {
	uint32_t headerSize;         /* The size of this header (40 bytes). */
	int32_t width;               /* The bitmap width in pixels. */
	int32_t height;              /* The bitmap height in pixels. */
	                             /* (A negative value denotes that the image
								  * is flipped.) */
	uint16_t colorPlanes;        /* The number of color planes; must be 1. */
	uint16_t bitsPerPixel;       /* The color depth of the image (1, 4, 8, 16,
	                              * 24, or 32). */
	BMP_COMPRESSION compression; /* The compression method being used. */
	uint32_t imageSize;          /* Size of the bitmap in bytes (unreliable).*/
	int32_t xRes;                /* The horizontal resolution (unreliable). */
	int32_t yRes;                /* The vertical resolution (unreliable). */
	uint32_t colorsUsed;         /* The number of colors in the color table,
	                              * or 0 to default to 2^n. */
	uint32_t colorsImportant;    /* Colors important for displaying bitmap,
	                              * or 0 when every color is equally important;
	                              * ignored. */
};

/*
 * OS/2 v1 Header
 */
struct BITMAP_CORE_HEADER {
	uint32_t headerSize;   /* The size of this header (12 bytes). */
	uint16_t width;        /* The bitmap width in pixels. */
	uint16_t height;       /* The bitmap height in pixels. */
	uint16_t colorPlanes;  /* The number of color planes; must be 1. */
	uint16_t bitsPerPixel; /* Color depth of the image (1, 4, 8, or 24). */
};

#pragma pack(pop) /* Let the compiler do what it wants now. */

/* BMP files are always saved in little endian format (x86), so we need to
 * convert them if we're not on a little endian machine (e.g., ARM & ppc). */

#if __BYTE_ORDER == __BIG_ENDIAN

/* Converts bitmap file header from to and from little endian, if and only if
 * host is big endian. */
static void convertBitmapFileHeader(struct BITMAP_FILE_HEADER *header)
{
	header->magic = swapLittleAndHost16(header->magic);
	swapLittleAndHost32(header->fileSize);
	swapLittleAndHost32(header->reserved);
	swapLittleAndHost32(header->imageOffset);
}

/* Converts bitmap info header from to and from little endian, if and only if
 * host is big endian. */
static void convertBitmapInfoHeader(struct BITMAP_INFO_HEADER *header)
{
	header->headerSize = swapLittleAndHost32(header->headerSize);
	header->width = swapLittleAndHost32(header->width);
	header->height = swapLittleAndHost32(header->height);
	header->colorPlanes = swapLittleAndHost16(header->colorPlanes);
	header->bitsPerPixel = swapLittleAndHost16(header->bitsPerPixel);
	header->compression = swapLittleAndHost32(header->compression);
	header->imageSize = swapLittleAndHost32(header->imageSize);
	header->xRes = swapLittleAndHost32(header->xRes);
	header->yRes = swapLittleAndHost32(header->yRes);
	header->colorsUsed = swapLittleAndHost32(header->colorsUsed);
	header->colorsImportant = swapLittleAndHost32(header->colorsImportant);
}

#elif __BYTE_ORDER == __LITTLE_ENDIAN
	/* No conversion necessary if we are already little endian. */
	#define convertBitmapFileHeader(header)
	#define convertBitmapInfoHeader(header)
#endif

/* Returns newly alloc'd image data from bitmap file. The current position of
 * the file must be at the start of the image before calling this. */
static uint8_t *readImageData(FILE *fp, size_t width, size_t height,
                              uint8_t bytesPerPixel, size_t bytewidth);

/* Copys image buffer from |bitmap| to |dest| in BGR format. */
static void copyBGRDataFromMMBitmap(MMBitmapRef bitmap, uint8_t *dest);

const char *MMBMPReadErrorString(MMIOError error)
{
	switch (error) {
		case kBMPAccessError:
			return "Could not open file";
		case kBMPInvalidKeyError:
			return "Not a BMP file";
		case kBMPUnsupportedHeaderError:
			return "Unsupported BMP header";
		case kBMPInvalidColorPanesError:
			return "Invalid number of color panes in BMP file";
		case kBMPUnsupportedColorDepthError:
			return "Unsupported color depth in BMP file";
		case kBMPUnsupportedCompressionError:
			return "Unsupported file compression in BMP file";
		case kBMPInvalidPixelDataError:
			return "Could not read BMP pixel data";
		default:
			return NULL;
	}
}

MMBitmapRef newMMBitmapFromBMP(const char *path, MMBMPReadError *err)
{
	FILE *fp;
	struct BITMAP_FILE_HEADER fileHeader = {0}; /* Initialize elements to 0. */
	struct BITMAP_INFO_HEADER dibHeader = {0};
	uint32_t headerSize = 0;
	uint8_t bytesPerPixel;
	size_t bytewidth;
	uint8_t *imageBuf;

	if ((fp = fopen(path, "rb")) == NULL) {
		if (err != NULL) *err = kBMPAccessError;
		return NULL;
	}

	/* Initialize error code to generic value. */
	if (err != NULL) *err = kBMPGenericError;

	if (fread(&fileHeader, sizeof(fileHeader), 1, fp) == 0) goto bail;

	/* Convert from little-endian if it's not already. */
	convertBitmapFileHeader(&fileHeader);

	/* First two bytes should always be 0x4D42. */
	if (fileHeader.magic != BMP_MAGIC) {
		if (err != NULL) *err = kBMPInvalidKeyError;
		goto bail;
	}

	/* Get header size. */
	if (fread(&headerSize, sizeof(headerSize), 1, fp) == 0) goto bail;
	headerSize = swapLittleAndHost32(headerSize);

	/* Back up before reading header. */
	if (fseek(fp, -(long)sizeof(headerSize), SEEK_CUR) < 0) goto bail;

	if (headerSize == 12) { /* OS/2 v1 header */
		struct BITMAP_CORE_HEADER coreHeader = {0};
		if (fread(&coreHeader, sizeof(coreHeader), 1, fp) == 0) goto bail;

		dibHeader.width = coreHeader.width;
		dibHeader.height = coreHeader.height;
		dibHeader.colorPlanes = coreHeader.colorPlanes;
		dibHeader.bitsPerPixel = coreHeader.bitsPerPixel;
	} else if (headerSize == 40 || headerSize == 108 || headerSize == 124) {
		/* Windows v3/v4/v5 header */
		/* Read only the common part (v3) and skip over the rest. */
		if (fread(&dibHeader, sizeof(dibHeader), 1, fp) == 0) goto bail;
	} else {
		if (err != NULL) *err = kBMPUnsupportedHeaderError;
		goto bail;
	}

	convertBitmapInfoHeader(&dibHeader);

	if (dibHeader.colorPlanes != 1) {
		if (err != NULL) *err = kBMPInvalidColorPanesError;
		goto bail;
	}

	/* Currently only 24-bit and 32-bit are supported. */
	if (dibHeader.bitsPerPixel != 24 && dibHeader.bitsPerPixel != 32) {
		if (err != NULL) *err = kBMPUnsupportedColorDepthError;
		goto bail;
	}

	if (dibHeader.compression != kBMP_RGB) {
		if (err != NULL) *err = kBMPUnsupportedCompressionError;
		goto bail;
	}

	/* This can happen because we don't fully parse Windows v4/v5 headers. */
	if (ftell(fp) != (long)fileHeader.imageOffset) {
		fseek(fp, fileHeader.imageOffset, SEEK_SET);
	}

	/* Get bytes per row, including padding. */
	bytesPerPixel = dibHeader.bitsPerPixel / 8;
	bytewidth = ADD_PADDING(dibHeader.width * bytesPerPixel);

	imageBuf = readImageData(fp, dibHeader.width, abs(dibHeader.height),
	                         bytesPerPixel, bytewidth);
	fclose(fp);

	if (imageBuf == NULL) {
		if (err != NULL) *err = kBMPInvalidPixelDataError;
		return NULL;
	}

	/* A negative height indicates that the image is flipped.
	 *
	 * We store our bitmaps as "flipped" according to the BMP format; i.e., (0, 0)
	 * is the top left, not bottom left. So we only need to flip the bitmap if
	 * the height is NOT negative. */
	if (dibHeader.height < 0) {
		dibHeader.height = -dibHeader.height;
	} else {
		flipBitmapData(imageBuf, dibHeader.width, dibHeader.height, bytewidth);
	}

	return createMMBitmap(imageBuf, dibHeader.width, dibHeader.height,
	                      bytewidth, (uint8_t)dibHeader.bitsPerPixel,
	                      bytesPerPixel);

bail:
	fclose(fp);
	return NULL;
}

uint8_t *createBitmapData(MMBitmapRef bitmap, size_t *len)
{
	/* BMP files are always aligned to 4 bytes. */
	const size_t bytewidth = ((bitmap->width * bitmap->bytesPerPixel) + 3) & ~3;

	const size_t imageSize = bytewidth * bitmap->height;
	struct BITMAP_FILE_HEADER *fileHeader;
	struct BITMAP_INFO_HEADER *dibHeader;

	/* Should always be 54. */
	const size_t imageOffset = sizeof(*fileHeader) + sizeof(*dibHeader);
	uint8_t *data;
	const size_t dataLen = imageOffset + imageSize;

	data = calloc(1, dataLen);
	if (data == NULL) return NULL;

	/* Save top header. */
	fileHeader = (struct BITMAP_FILE_HEADER *)data;
	fileHeader->magic = BMP_MAGIC;
	fileHeader->fileSize = (uint32_t)(sizeof(*dibHeader) + imageSize);
	fileHeader->imageOffset = (uint32_t)imageOffset;

	/* BMP files are always stored as little-endian, so we need to convert back
	 * if necessary. */
	convertBitmapFileHeader(fileHeader);

	/* Copy Windows v3 header. */
	dibHeader = (struct BITMAP_INFO_HEADER *)(data + sizeof(*fileHeader));
	dibHeader->headerSize = sizeof(*dibHeader); /* Should always be 40. */
	dibHeader->width = (int32_t)bitmap->width;
	dibHeader->height = -(int32_t)bitmap->height; /* Our bitmaps are "flipped". */
	dibHeader->colorPlanes = 1;
	dibHeader->bitsPerPixel = bitmap->bitsPerPixel;
	dibHeader->compression = kBMP_RGB; /* Don't save with compression. */
	dibHeader->imageSize = (uint32_t)imageSize;

	convertBitmapInfoHeader(dibHeader);

	/* Lastly, copy the pixel data. */
	copyBGRDataFromMMBitmap(bitmap, data + imageOffset);

	if (len != NULL) *len = dataLen;
	return data;
}

int saveMMBitmapAsBMP(MMBitmapRef bitmap, const char *path)
{
	FILE *fp;
	size_t dataLen;
	uint8_t *data;

	if ((fp = fopen(path, "wb")) == NULL) return -1;

	if ((data = createBitmapData(bitmap, &dataLen)) == NULL) {
		fclose(fp);
		return -1;
	}

	if (fwrite(data, dataLen, 1, fp) == 0) {
		free(data);
		fclose(fp);
		return -1;
	}

	free(data);
	fclose(fp);
	return 0;
}

static uint8_t *readImageData(FILE *fp, size_t width, size_t height,
                              uint8_t bytesPerPixel, size_t bytewidth)
{
	size_t imageSize = bytewidth * height;
	uint8_t *imageBuf = calloc(1, imageSize);

	if (MMRGB_IS_BGR && (bytewidth % 4) == 0) { /* No conversion needed. */
		if (fread(imageBuf, imageSize, 1, fp) == 0) {
			free(imageBuf);
			return NULL;
		}
	} else { /* Convert from BGR with 4-byte alignment. */
		uint8_t *row = malloc(bytewidth);
		size_t y;
		const size_t bmp_bytewidth = (width * bytesPerPixel + 3) & ~3;

		if (row == NULL) return NULL;
		assert(bmp_bytewidth <= bytewidth);

		/* Read image data row by row. */
		for (y = 0; y < height; ++y) {
			const size_t rowOffset = y * bytewidth;
			size_t x;
			uint8_t *rowptr = row;
			if (fread(row, bmp_bytewidth, 1, fp) == 0) {
				free(imageBuf);
				free(row);
				return NULL;
			}

			for (x = 0; x < width; ++x) {
				const size_t colOffset = x * bytesPerPixel;
				MMRGBColor *color = (MMRGBColor *)(imageBuf +
				                                   rowOffset + colOffset);

				/* BMP files are stored in BGR format. */
				color->blue = rowptr[0];
				color->green = rowptr[1];
				color->red = rowptr[2];
				rowptr += bytesPerPixel;
			}
		}

		free(row);
	}

	return imageBuf;
}

static void copyBGRDataFromMMBitmap(MMBitmapRef bitmap, uint8_t *dest)
{
	if (MMRGB_IS_BGR && (bitmap->bytewidth % 4) == 0) { /* No conversion needed. */
		memcpy(dest, bitmap->imageBuffer, bitmap->bytewidth * bitmap->height);
	} else { /* Convert to RGB with other-than-4-byte alignment. */
		const size_t bytewidth = (bitmap->width * bitmap->bytesPerPixel + 3) & ~3;
		size_t y;

		/* Copy image data row by row. */
		for (y = 0; y < bitmap->height; ++y) {
			uint8_t *rowptr = dest + (y * bytewidth);
			size_t x;
			for (x = 0; x < bitmap->width; ++x) {
				MMRGBColor *color = MMRGBColorRefAtPoint(bitmap, x, y);

				/* BMP files are stored in BGR format. */
				rowptr[0] = color->blue;
				rowptr[1] = color->green;
				rowptr[2] = color->red;

				rowptr += bitmap->bytesPerPixel;
			}
		}
	}
}

/* Perform an in-place swap from Quadrant 1 to Quadrant III format (upside-down
 * PostScript/GL to right side up QD/CG raster format) We do this in-place,
 * which requires more copying, but will touch only half the pages.
 *
 * This is blatantly copied from Apple's glGrab example code. */
void flipBitmapData(void *data, size_t width, size_t height, size_t bytewidth)
{
	size_t top, bottom;
	void *topP;
	void *bottomP;
	void *tempbuf;

	if (height <= 1) return; /* No flipping necessary if height is <= 1. */

	top = 0;
	bottom = height - 1;
	tempbuf = malloc(bytewidth);
	if (tempbuf == NULL) return;

	while (top < bottom) {
		topP = (void *)((top * bytewidth) + (intptr_t)data);
		bottomP = (void *)((bottom * bytewidth) + (intptr_t)data);

		/* Save and swap scanlines.
		 * Does a simple in-place exchange with a temp buffer. */
		memcpy(tempbuf, topP, bytewidth);
		memcpy(topP, bottomP, bytewidth);
		memcpy(bottomP, tempbuf, bytewidth);

		++top;
		--bottom;
	}
	free(tempbuf);
}
