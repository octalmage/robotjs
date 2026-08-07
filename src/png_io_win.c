#define COBJMACROS

#include "png_io.h"
#include "os.h"

#include <objidl.h>
#include <wincodec.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static HRESULT createWICFactory(IWICImagingFactory **factory, int *uninitialize)
{
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	*factory = NULL;
	*uninitialize = SUCCEEDED(result);
	if (FAILED(result) && result != RPC_E_CHANGED_MODE) return result;

	result = CoCreateInstance(&CLSID_WICImagingFactory,
	                          NULL,
	                          CLSCTX_INPROC_SERVER,
	                          &IID_IWICImagingFactory,
	                          (void **)factory);
	if (FAILED(result) && *uninitialize) {
		CoUninitialize();
		*uninitialize = 0;
	}
	return result;
}

static void destroyWICFactory(IWICImagingFactory *factory, int uninitialize)
{
	if (factory != NULL) IWICImagingFactory_Release(factory);
	if (uninitialize) CoUninitialize();
}

static WCHAR *widePathFromUTF8(const char *path)
{
	WCHAR *widePath;
	int length;

	length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
	                             path, -1, NULL, 0);
	if (length == 0 || (size_t)length > SIZE_MAX / sizeof(WCHAR)) return NULL;

	widePath = malloc((size_t)length * sizeof(WCHAR));
	if (widePath == NULL) return NULL;

	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
	                        path, -1, widePath, length) == 0) {
		free(widePath);
		return NULL;
	}
	return widePath;
}

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
	IWICImagingFactory *factory = NULL;
	IWICBitmapDecoder *decoder = NULL;
	IWICBitmapFrameDecode *frame = NULL;
	IWICFormatConverter *converter = NULL;
	WCHAR *widePath = NULL;
	uint8_t *bitmapData = NULL;
	MMBitmapRef bitmap = NULL;
	GUID containerFormat;
	UINT width = 0;
	UINT height = 0;
	UINT bytewidth;
	UINT bufferSize;
	int uninitialize = 0;
	HRESULT result;

	if (err != NULL) *err = kPNGGenericError;

	widePath = widePathFromUTF8(path);
	if (widePath == NULL) goto bail;

	result = createWICFactory(&factory, &uninitialize);
	if (FAILED(result)) goto bail;

	result = IWICImagingFactory_CreateDecoderFromFilename(
		factory, widePath, NULL, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(result)) {
		if (err != NULL) *err = kPNGAccessError;
		goto bail;
	}

	result = IWICBitmapDecoder_GetContainerFormat(decoder, &containerFormat);
	if (FAILED(result)) {
		if (err != NULL) *err = kPNGReadError;
		goto bail;
	}
	if (memcmp(&containerFormat, &GUID_ContainerFormatPng, sizeof(GUID)) != 0) {
		if (err != NULL) *err = kPNGInvalidHeaderError;
		goto bail;
	}

	result = IWICBitmapDecoder_GetFrame(decoder, 0, &frame);
	if (FAILED(result) ||
	    FAILED(IWICBitmapFrameDecode_GetSize(frame, &width, &height))) {
		if (err != NULL) *err = kPNGReadError;
		goto bail;
	}
	if (width == 0 || height == 0 || width > (UINT_MAX - 3U) / 3U) {
		if (err != NULL) *err = kPNGReadError;
		goto bail;
	}

	bytewidth = (width * 3U + 3U) & ~3U;
	if (height > UINT_MAX / bytewidth) {
		if (err != NULL) *err = kPNGReadError;
		goto bail;
	}
	bufferSize = bytewidth * height;
	bitmapData = calloc(1, bufferSize);
	if (bitmapData == NULL) goto bail;

	result = IWICImagingFactory_CreateFormatConverter(factory, &converter);
	if (FAILED(result) ||
	    FAILED(IWICFormatConverter_Initialize(
		converter, (IWICBitmapSource *)frame, &GUID_WICPixelFormat24bppBGR,
		WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom)) ||
	    FAILED(IWICFormatConverter_CopyPixels(
		converter, NULL, bytewidth, bufferSize, bitmapData))) {
		if (err != NULL) *err = kPNGReadError;
		goto bail;
	}

	bitmap = createMMBitmap(bitmapData, width, height, bytewidth, 24, 3);
	if (bitmap != NULL) bitmapData = NULL;

bail:
	free(bitmapData);
	free(widePath);
	if (converter != NULL) IWICFormatConverter_Release(converter);
	if (frame != NULL) IWICBitmapFrameDecode_Release(frame);
	if (decoder != NULL) IWICBitmapDecoder_Release(decoder);
	destroyWICFactory(factory, uninitialize);
	return bitmap;
}

static uint8_t *createPackedBGRData(MMBitmapRef bitmap,
                                    UINT *stride,
                                    UINT *bufferSize,
                                    int *mustFree)
{
	uint8_t *data;
	size_t packedStride;
	size_t packedSize;
	size_t x;
	size_t y;

	if (bitmap == NULL || bitmap->width == 0 || bitmap->height == 0 ||
	    bitmap->width > UINT_MAX / 3U) {
		return NULL;
	}

	packedStride = bitmap->width * 3U;
	if (bitmap->height > UINT_MAX / packedStride) return NULL;
	packedSize = packedStride * bitmap->height;
	*stride = (UINT)packedStride;
	*bufferSize = (UINT)packedSize;

	if (MMRGB_IS_BGR && bitmap->bytesPerPixel == 3 &&
	    bitmap->bytewidth == packedStride) {
		*mustFree = 0;
		return bitmap->imageBuffer;
	}

	data = malloc(packedSize);
	if (data == NULL) return NULL;

	for (y = 0; y < bitmap->height; ++y) {
		uint8_t *output = data + y * packedStride;
		for (x = 0; x < bitmap->width; ++x) {
			MMRGBColor *color = MMRGBColorRefAtPoint(bitmap, x, y);
			*output++ = color->blue;
			*output++ = color->green;
			*output++ = color->red;
		}
	}

	*mustFree = 1;
	return data;
}

static int writePNGToStream(MMBitmapRef bitmap,
                            IWICImagingFactory *factory,
                            IStream *stream)
{
	IWICBitmapEncoder *encoder = NULL;
	IWICBitmapFrameEncode *frame = NULL;
	IPropertyBag2 *properties = NULL;
	uint8_t *bitmapData = NULL;
	WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat24bppBGR;
	UINT stride = 0;
	UINT bufferSize = 0;
	int mustFree = 0;
	int status = -1;
	HRESULT result;

	bitmapData = createPackedBGRData(bitmap, &stride, &bufferSize, &mustFree);
	if (bitmapData == NULL || bitmap->width > UINT_MAX || bitmap->height > UINT_MAX) {
		goto bail;
	}

	result = IWICImagingFactory_CreateEncoder(
		factory, &GUID_ContainerFormatPng, NULL, &encoder);
	if (FAILED(result) ||
	    FAILED(IWICBitmapEncoder_Initialize(
		encoder, stream, WICBitmapEncoderNoCache)) ||
	    FAILED(IWICBitmapEncoder_CreateNewFrame(
		encoder, &frame, &properties)) ||
	    FAILED(IWICBitmapFrameEncode_Initialize(frame, properties)) ||
	    FAILED(IWICBitmapFrameEncode_SetSize(
		frame, (UINT)bitmap->width, (UINT)bitmap->height)) ||
	    FAILED(IWICBitmapFrameEncode_SetPixelFormat(frame, &pixelFormat)) ||
	    memcmp(&pixelFormat, &GUID_WICPixelFormat24bppBGR, sizeof(GUID)) != 0 ||
	    FAILED(IWICBitmapFrameEncode_WritePixels(
		frame, (UINT)bitmap->height, stride, bufferSize, bitmapData)) ||
	    FAILED(IWICBitmapFrameEncode_Commit(frame)) ||
	    FAILED(IWICBitmapEncoder_Commit(encoder))) {
		goto bail;
	}

	status = 0;

bail:
	if (mustFree) free(bitmapData);
	if (properties != NULL) IPropertyBag2_Release(properties);
	if (frame != NULL) IWICBitmapFrameEncode_Release(frame);
	if (encoder != NULL) IWICBitmapEncoder_Release(encoder);
	return status;
}

int saveMMBitmapAsPNG(MMBitmapRef bitmap, const char *path)
{
	IWICImagingFactory *factory = NULL;
	IWICStream *stream = NULL;
	WCHAR *widePath = NULL;
	int uninitialize = 0;
	int status = -1;
	HRESULT result;

	widePath = widePathFromUTF8(path);
	if (widePath == NULL) goto bail;

	result = createWICFactory(&factory, &uninitialize);
	if (FAILED(result) ||
	    FAILED(IWICImagingFactory_CreateStream(factory, &stream)) ||
	    FAILED(IWICStream_InitializeFromFilename(stream, widePath, GENERIC_WRITE))) {
		goto bail;
	}

	status = writePNGToStream(bitmap, factory, (IStream *)stream);

bail:
	free(widePath);
	if (stream != NULL) IWICStream_Release(stream);
	destroyWICFactory(factory, uninitialize);
	return status;
}

uint8_t *createPNGData(MMBitmapRef bitmap, size_t *len)
{
	IWICImagingFactory *factory = NULL;
	IStream *stream = NULL;
	HGLOBAL global = NULL;
	STATSTG streamInfo;
	void *source;
	uint8_t *data = NULL;
	size_t size;
	int uninitialize = 0;
	HRESULT result;

	assert(bitmap != NULL);
	assert(len != NULL);
	*len = 0;

	result = createWICFactory(&factory, &uninitialize);
	if (FAILED(result) ||
	    FAILED(CreateStreamOnHGlobal(NULL, TRUE, &stream)) ||
	    writePNGToStream(bitmap, factory, stream) != 0 ||
	    FAILED(IStream_Stat(stream, &streamInfo, STATFLAG_NONAME)) ||
	    FAILED(GetHGlobalFromStream(stream, &global))) {
		goto bail;
	}

#if SIZE_MAX <= UINT_MAX
	if (streamInfo.cbSize.HighPart != 0 ||
	    streamInfo.cbSize.LowPart > SIZE_MAX) {
		goto bail;
	}
	size = (size_t)streamInfo.cbSize.LowPart;
#else
	if (streamInfo.cbSize.QuadPart > SIZE_MAX) goto bail;
	size = (size_t)streamInfo.cbSize.QuadPart;
#endif
	if (size == 0) goto bail;

	source = GlobalLock(global);
	if (source == NULL) goto bail;
	data = malloc((size_t)size);
	if (data != NULL) {
		memcpy(data, source, (size_t)size);
		*len = (size_t)size;
	}
	GlobalUnlock(global);

bail:
	if (stream != NULL) IStream_Release(stream);
	destroyWICFactory(factory, uninitialize);
	return data;
}
