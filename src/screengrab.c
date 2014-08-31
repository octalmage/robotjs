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
	#include <string.h>
#endif

#if defined(IS_MACOSX)

/* Helper functions (documented below). */
static CGLContextObj createFullScreenCGLContext(CGOpenGLDisplayMask displayMask);
static void destroyFullScreenCGLContext(CGLContextObj glContext);

static uint8_t *createBufferFromCurrentCGLContext(GLint x,
                                                  GLint y,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  size_t bytewidth);

#endif

MMBitmapRef copyMMBitmapFromDisplayInRect(MMRect rect)
{
#if defined(IS_MACOSX)
	/* The following is a very modified version of the glGrab code example
	 * given by Apple (as are some of the convenience functions called). */
	size_t bytewidth;
	uint8_t bitsPerPixel, bytesPerPixel;
	uint8_t *buffer;

	/* Build OpenGL context of entire screen */
	CGDirectDisplayID displayID = CGMainDisplayID();
	CGOpenGLDisplayMask mask = CGDisplayIDToOpenGLDisplayMask(displayID);
	CGLContextObj glContext = createFullScreenCGLContext(mask);
	if (glContext == NULL) return NULL;

	/* TODO: CGDisplayBitsPerPixel() is deprecated in Snow Leopard; I'm not
	 * sure of the replacement function. */
	bitsPerPixel = (uint8_t)CGDisplayBitsPerPixel(displayID);
	bytesPerPixel = bitsPerPixel / 8;

	/* Align width to padding. */
	bytewidth = ADD_PADDING(rect.size.width * bytesPerPixel);

	/* Convert Quartz point to postscript point. */
	rect.origin.y = CGDisplayPixelsHigh(displayID) - rect.origin.y - rect.size.height;

	/* Extract buffer from context */
	buffer = createBufferFromCurrentCGLContext((GLint)rect.origin.x,
	                                           (GLint)rect.origin.y,
	                                           (GLsizei)rect.size.width,
	                                           (GLsizei)rect.size.height,
	                                           bytewidth);
	/* Reset and release GL context */
	destroyFullScreenCGLContext(glContext);
	if (buffer == NULL) return NULL;

	/* Convert from OpenGL (origin at bottom left) to Quartz (origin at top
	 * left) coordinate system. */
	flipBitmapData(buffer, rect.size.width, rect.size.height, bytewidth);

	return createMMBitmap(buffer, rect.size.width, rect.size.height, bytewidth,
	                      bitsPerPixel, bytesPerPixel);
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
	BITMAPINFO bi;

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

	/* Copy the data into a bitmap struct. */
	if ((screenMem = CreateCompatibleDC(screen)) == NULL ||
	    SelectObject(screenMem, dib) == NULL ||
	    !BitBlt(screenMem, 
	            (int)rect.origin.x, 
	            (int)rect.origin.y, 
	            (int)rect.size.width,
	            (int)rect.size.height, screen, 0, 0, SRCCOPY)) {
		/* Error copying data. */
		ReleaseDC(NULL, screen);
		DeleteObject(dib);
		if (screenMem != NULL) DeleteDC(screenMem);

		return NULL;
	}

	bitmap = createMMBitmap(NULL,
	                        rect.size.width,
	                        rect.size.height,
	                        4 * rect.size.width,
	                        (uint8_t)bi.bmiHeader.biBitCount, 
	                        4);

	/* Copy the data to our pixel buffer. */
	if (bitmap != NULL) {
		bitmap->imageBuffer = malloc(bitmap->bytewidth * bitmap->height);
		memcpy(bitmap->imageBuffer, data, bitmap->bytewidth * bitmap->height);
	}

	ReleaseDC(NULL, screen);
	DeleteObject(dib);
	DeleteDC(screenMem);

	return bitmap;
#endif
}

#if defined(IS_MACOSX)

/* Creates and returns a full-screen OpenGL graphics context (to be
 * released/destroyed by caller).
 *
 * To clean up the returned context use destroyFullScreenCGLContext(); */
static CGLContextObj createFullScreenCGLContext(CGOpenGLDisplayMask displayMask)
{
	CGLContextObj glContext = NULL;
	CGLPixelFormatObj pix;
	GLint npix;
	CGLPixelFormatAttribute attribs[4];

	attribs[0] = kCGLPFAFullScreen;
	attribs[1] = kCGLPFADisplayMask;
	attribs[2] = displayMask;
	attribs[3] = (CGLPixelFormatAttribute)0;

	CGLChoosePixelFormat(attribs, &pix, &npix);
	CGLCreateContext(pix, NULL, &glContext);

	/* The pixel format is no longer needed, so destroy it. */
	CGLDestroyPixelFormat(pix);

	if (glContext == NULL) return NULL;

	/* Set our context as the current OpenGL context. */
	CGLSetCurrentContext(glContext);

	/* Set full-screen mode. */
	CGLSetFullScreen(glContext);

	/* Select front buffer as our source for pixel data. */
	glReadBuffer(GL_FRONT);

	/* Finish previous OpenGL commands before continuing. */
	glFinish();

	if (glGetError() != GL_NO_ERROR) return NULL;

	return glContext;
}

/* Cleans up CGLContext created by createFullScreenCGLContext(); */
static void destroyFullScreenCGLContext(CGLContextObj glContext)
{
	glPopClientAttrib(); /* Clear attributes previously set. */
	CGLSetCurrentContext(NULL); /* Reset context. */
	CGLClearDrawable(glContext); /* Disassociate from full-screen. */
	CGLDestroyContext(glContext); /* Release memory. */
}

/* Returns newly malloc'd bitmap (to be freed by caller). */
static uint8_t *createBufferFromCurrentCGLContext(GLint x,
                                                  GLint y,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  size_t bytewidth)
{
	uint8_t *data = NULL;

	/* For extra safety, save & restore OpenGL states that are changed. */
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

	glPixelStorei(GL_PACK_ALIGNMENT, BYTE_ALIGN); /* Force alignment. */
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	/* Allocate size for bitmap */
	data = malloc(bytewidth * height);
	if (data == NULL) return NULL;

	/* Read the OpenGL frame into our buffer */
	glReadPixels(x, y, width, height,
	             MMRGB_IS_BGR ? GL_BGRA : GL_RGBA,
#if __BYTE_ORDER == __BIG_ENDIAN
	             GL_UNSIGNED_INT_8_8_8_8, /* Non-native format (little-endian) */
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	             GL_UNSIGNED_INT_8_8_8_8_REV, /* Native format */
#endif
	             data);

	return data;
}

#endif
