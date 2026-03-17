#include "screengrab.h"
#include "bmp_io.h"
#include "endian.h"
#include <limits.h>
#include <stdlib.h> /* malloc() */

#if defined(IS_MACOSX)
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
	#include <ApplicationServices/ApplicationServices.h>
#elif defined(USE_X11)
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include "xdisplay.h"
#elif defined(IS_WINDOWS)
	#include <string.h>
#endif

#if defined(IS_MACOSX)
#elif defined(IS_WINDOWS)
static void destroyMMBitmapWindowsDIB(char *bitmapBuffer, void *hint)
{
	if (hint != NULL) {
		DeleteObject((HGDIOBJ)hint);
	}
}
#endif

#if !defined(IS_MACOSX)
static bool getDisplayCaptureOrigin(MMDisplay display, MMRect rect,
                                    int32_t *absoluteX, int32_t *absoluteY)
{
	int64_t sourceX;
	int64_t sourceY;

	if (absoluteX == NULL || absoluteY == NULL ||
	    rect.origin.x > (size_t)INT32_MAX || rect.origin.y > (size_t)INT32_MAX) {
		return false;
	}

	sourceX = (int64_t)display.x + (int64_t)rect.origin.x;
	sourceY = (int64_t)display.y + (int64_t)rect.origin.y;

	if (sourceX < INT32_MIN || sourceX > INT32_MAX ||
	    sourceY < INT32_MIN || sourceY > INT32_MAX) {
		return false;
	}

	*absoluteX = (int32_t)sourceX;
	*absoluteY = (int32_t)sourceY;
	return true;
}
#endif

MMBitmapRef copyMMBitmapFromDisplayInRectOnDisplay(MMDisplay display, MMRect rect)
{
#if defined(IS_MACOSX)

	MMBitmapRef bitmap = NULL;
	CGContextRef context = NULL;
	CGColorSpaceRef colorSpace = NULL;
	uint8_t *buffer = NULL;
	const size_t width = rect.size.width;
	const size_t height = rect.size.height;
	const size_t bytesPerPixel = 4;
	const size_t bytesPerRow = width * bytesPerPixel;

	CGDirectDisplayID displayID = (CGDirectDisplayID)display.id;

	CGImageRef image = CGDisplayCreateImageForRect(displayID,
		CGRectMake(rect.origin.x,
			rect.origin.y,
			rect.size.width,
			rect.size.height));

	if (!image) { return NULL; }

	colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	if (colorSpace == NULL) {
		CGImageRelease(image);
		return NULL;
	}

	buffer = calloc(height, bytesPerRow);
	if (buffer == NULL) {
		CGColorSpaceRelease(colorSpace);
		CGImageRelease(image);
		return NULL;
	}

	context = CGBitmapContextCreate(buffer,
	                                width,
	                                height,
	                                8,
	                                bytesPerRow,
	                                colorSpace,
	                                kCGImageAlphaNoneSkipFirst |
	                                kCGBitmapByteOrder32Little);
	if (context == NULL) {
		free(buffer);
		CGColorSpaceRelease(colorSpace);
		CGImageRelease(image);
		return NULL;
	}

	CGContextSetBlendMode(context, kCGBlendModeCopy);
	CGContextSetInterpolationQuality(context, kCGInterpolationNone);
	CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

	bitmap = createMMBitmap(buffer,
	                        width,
	                        height,
	                        bytesPerRow,
	                        32,
	                        4);
	if (bitmap == NULL) {
		free(buffer);
	}

	CGContextRelease(context);
	CGColorSpaceRelease(colorSpace);

	CGImageRelease(image);

	return bitmap;

#elif defined(USE_X11)
	MMBitmapRef bitmap;
	int32_t sourceX;
	int32_t sourceY;
	Display *xDisplay = XGetMainDisplay();

	if (xDisplay == NULL ||
	    !getDisplayCaptureOrigin(display, rect, &sourceX, &sourceY)) {
		return NULL;
	}

	XImage *image = XGetImage(xDisplay,
	                          XDefaultRootWindow(xDisplay),
	                          sourceX,
	                          sourceY,
	                          (unsigned int)rect.size.width,
	                          (unsigned int)rect.size.height,
	                          AllPlanes, ZPixmap);
	if (image == NULL) return NULL;

	bitmap = createMMBitmap((uint8_t *)image->data,
	                        rect.size.width,
	                        rect.size.height,
	                        (size_t)image->bytes_per_line,
	                        (uint8_t)image->bits_per_pixel,
	                        (uint8_t)image->bits_per_pixel / 8);
	image->data = NULL; /* Steal ownership of bitmap data so we don't have to
	                     * copy it. */
	XDestroyImage(image);

	return bitmap;
#elif defined(IS_WINDOWS)
	MMBitmapRef bitmap;
	void *data;
	HDC screen = NULL, screenMem = NULL;
	HBITMAP dib;
	HGDIOBJ previousObject = NULL;
	BITMAPINFO bi;
	int32_t sourceX;
	int32_t sourceY;

	if (!getDisplayCaptureOrigin(display, rect, &sourceX, &sourceY)) {
		return NULL;
	}

	/* Initialize bitmap info. */
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
   	bi.bmiHeader.biWidth = (long)rect.size.width;
   	bi.bmiHeader.biHeight = -(long)rect.size.height; /* Non-cartesian, please */
   	bi.bmiHeader.biPlanes = 1;
   	bi.bmiHeader.biBitCount = 32;
   	bi.bmiHeader.biCompression = BI_RGB;
   	bi.bmiHeader.biSizeImage = (DWORD)(4 * rect.size.width * rect.size.height);
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	screen = GetDC(NULL); /* Get entire screen */
	if (screen == NULL) return NULL;

	/* Get screen data in display device context. */
   	dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &data, NULL, 0);
	if (dib == NULL || data == NULL) {
		if (dib != NULL) DeleteObject(dib);
		ReleaseDC(NULL, screen);
		return NULL;
	}

	/* Copy the data into a bitmap struct. */
	if ((screenMem = CreateCompatibleDC(screen)) == NULL ||
	    (previousObject = SelectObject(screenMem, dib)) == NULL ||
	    !BitBlt(screenMem,
	            (int)0,
	            (int)0,
	            (int)rect.size.width,
	            (int)rect.size.height,
				screen,
				sourceX,
				sourceY,
				SRCCOPY)) {
		
		/* Error copying data. */
		ReleaseDC(NULL, screen);
		DeleteObject(dib);
		if (screenMem != NULL) DeleteDC(screenMem);

		return NULL;
	}

	bitmap = createMMBitmapWithCleanup((uint8_t *)data,
	                                   rect.size.width,
	                                   rect.size.height,
	                                   4 * rect.size.width,
	                                   (uint8_t)bi.bmiHeader.biBitCount,
	                                   4,
	                                   destroyMMBitmapWindowsDIB,
	                                   dib);
	if (previousObject != NULL) {
		SelectObject(screenMem, previousObject);
	}
	if (bitmap == NULL) {
		DeleteObject(dib);
	}

	ReleaseDC(NULL, screen);
	DeleteDC(screenMem);

	return bitmap;
#endif
}

MMBitmapRef copyMMBitmapFromDisplayInRect(MMRect rect)
{
#if defined(IS_MACOSX)
	MMDisplay display;
	CGDirectDisplayID displayID = CGMainDisplayID();

	display.id = displayID;
	display.x = 0;
	display.y = 0;
	display.width = (size_t)CGDisplayPixelsWide(displayID);
	display.height = (size_t)CGDisplayPixelsHigh(displayID);
	display.isMain = true;

	return copyMMBitmapFromDisplayInRectOnDisplay(display, rect);
#else
	MMDisplay display;
	MMSize displaySize = getMainDisplaySize();

	display.id = 0;
	display.x = 0;
	display.y = 0;
	display.width = displaySize.width;
	display.height = displaySize.height;
	display.isMain = true;

	return copyMMBitmapFromDisplayInRectOnDisplay(display, rect);
#endif
}
