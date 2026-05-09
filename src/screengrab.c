#include "screengrab.h"
#include "bmp_io.h"
#include "endian.h"
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
	#include <stdio.h>
	#include <string.h>
#endif

#if defined(IS_WINDOWS)
MMBitmapRef copyMMBitmapFromDisplayInRectWindowsGpu(MMRect rect);
const char *copyMMBitmapFromDisplayInRectWindowsGpuLastError(void);

typedef HANDLE (WINAPI *MMSetThreadDpiAwarenessContextFunc)(HANDLE);

static MMSetThreadDpiAwarenessContextFunc getSetThreadDpiAwarenessContext(void)
{
	HMODULE user32 = GetModuleHandleA("user32.dll");
	if (user32 == NULL) return NULL;

	return (MMSetThreadDpiAwarenessContextFunc)GetProcAddress(user32,
	                                                         "SetThreadDpiAwarenessContext");
}

static void destroyMMBitmapWindowsDIB(char *bitmapBuffer, void *hint)
{
	if (hint != NULL) {
		DeleteObject((HGDIOBJ)hint);
	}
}

static void logWindowsGpuCaptureFailure(void)
{
	static char previousMessage[256];
	const char *message = copyMMBitmapFromDisplayInRectWindowsGpuLastError();
	if (message == NULL || message[0] == '\0') {
		message = "unknown error";
	}
	if (strcmp(previousMessage, message) != 0) {
		fprintf(stderr, "robotjs: Windows GPU screen capture failed (%s); falling back to GDI\n",
		        message);
		strncpy(previousMessage, message, sizeof(previousMessage) - 1);
		previousMessage[sizeof(previousMessage) - 1] = '\0';
	}
}
#endif

MMBitmapRef copyMMBitmapFromDisplayInRect(MMRect rect)
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

	CGDirectDisplayID displayID = CGMainDisplayID();

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

	/* RobotJS bitmaps use a top-left origin. Flip the Quartz context so the
	 * captured image keeps that coordinate system after color conversion. */
	CGContextTranslateCTM(context, 0, height);
	CGContextScaleCTM(context, 1, -1);
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

	Display *display = XOpenDisplay(NULL);
	XImage *image = XGetImage(display,
	                          XDefaultRootWindow(display),
	                          (int)rect.origin.x,
	                          (int)rect.origin.y,
	                          (unsigned int)rect.size.width,
	                          (unsigned int)rect.size.height,
	                          AllPlanes, ZPixmap);
	XCloseDisplay(display);
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
	MMSetThreadDpiAwarenessContextFunc setThreadDpiAwarenessContext;
	HANDLE previousDpiContext = NULL;
	const int srcX = 0;
	const int srcY = 0;
	int srcWidth;
	int srcHeight;
	const int destWidth = (int)rect.size.width;
	const int destHeight = (int)rect.size.height;

	if (rect.size.width == 0 || rect.size.height == 0) {
		return NULL;
	}

	bitmap = copyMMBitmapFromDisplayInRectWindowsGpu(rect);
	if (bitmap != NULL) {
		return bitmap;
	}
	logWindowsGpuCaptureFailure();

	setThreadDpiAwarenessContext = getSetThreadDpiAwarenessContext();
	if (setThreadDpiAwarenessContext != NULL) {
		previousDpiContext = setThreadDpiAwarenessContext((HANDLE)-4);
	}

	screen = GetDC(NULL); /* Get entire screen */
	if (screen == NULL) {
		if (setThreadDpiAwarenessContext != NULL && previousDpiContext != NULL) {
			setThreadDpiAwarenessContext(previousDpiContext);
		}
		return NULL;
	}
	srcWidth = GetSystemMetrics(SM_CXSCREEN);
	srcHeight = GetSystemMetrics(SM_CYSCREEN);

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

	/* Get screen data in display device context. */
	dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &data, NULL, 0);
	if (dib == NULL || data == NULL) {
		if (dib != NULL) DeleteObject(dib);
		ReleaseDC(NULL, screen);
		if (setThreadDpiAwarenessContext != NULL && previousDpiContext != NULL) {
			setThreadDpiAwarenessContext(previousDpiContext);
		}
		return NULL;
	}

	/* Copy the full physical display into the destination-sized bitmap. */
	if ((screenMem = CreateCompatibleDC(screen)) == NULL ||
	    (previousObject = SelectObject(screenMem, dib)) == NULL ||
	    SetStretchBltMode(screenMem, COLORONCOLOR) == 0 ||
	    !StretchBlt(screenMem,
	            0,
	            0,
	            destWidth,
	            destHeight,
				screen,
				srcX,
				srcY,
				srcWidth,
				srcHeight,
				SRCCOPY)) {
		
		/* Error copying data. */
		if (previousObject != NULL) {
			SelectObject(screenMem, previousObject);
		}
		ReleaseDC(NULL, screen);
		DeleteObject(dib);
		if (screenMem != NULL) DeleteDC(screenMem);
		if (setThreadDpiAwarenessContext != NULL && previousDpiContext != NULL) {
			setThreadDpiAwarenessContext(previousDpiContext);
		}

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
	if (bitmap == NULL) {
		DeleteObject(dib);
	}
	if (previousObject != NULL) {
		SelectObject(screenMem, previousObject);
	}

	ReleaseDC(NULL, screen);
	DeleteDC(screenMem);
	if (setThreadDpiAwarenessContext != NULL && previousDpiContext != NULL) {
		setThreadDpiAwarenessContext(previousDpiContext);
	}

	return bitmap;
#endif
}
