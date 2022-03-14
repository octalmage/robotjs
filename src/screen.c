#include "screen.h"
#include "os.h"

#if defined(IS_MACOSX)
	#include <ApplicationServices/ApplicationServices.h>
#elif defined(USE_X11)
	#include <X11/Xlib.h>
	#include "xdisplay.h"
#endif

#if defined(IS_MACOSX)
uint32_t getIDOfDisplayWithRect(MMRect rect)
{
	CGError cgErr;
	CGDisplayCount displayCount;
	CGDisplayCount maxDisplays = 1;
	CGDirectDisplayID onlineDisplays[1];
	CGRect cgRect = CGRectMake(rect.origin.x,
														 rect.origin.y,
														 rect.size.width,
														 rect.size.height);

	// If CGGetOnlineDisplayList() is not called, CGGetDisplaysWithRect() fails
	CGGetOnlineDisplayList(maxDisplays, onlineDisplays, &displayCount);
	cgErr = CGGetDisplaysWithRect(cgRect, maxDisplays, onlineDisplays, &displayCount);

	if (cgErr != kCGErrorSuccess)
	{
		fprintf(stderr, "CGGetDisplaysWithRect: error %d.\n", cgErr);
		exit(1);
	}
	if (displayCount == 0)
	{
		fprintf(stderr, 
						"No display with rect with origin (%d, %d), width %d and height %d.\n", 
						(int) cgRect.origin.x,
						(int) cgRect.origin.y,
						(int) cgRect.size.width,
						(int) cgRect.size.height);
		exit(1);
	}

	return onlineDisplays[0];
};
#endif

MMSignedSize getMainDisplaySize(void)
{
#if defined(IS_MACOSX)
	CGDirectDisplayID displayID = CGMainDisplayID();
	return MMSignedSizeMake(CGDisplayPixelsWide(displayID),
	                  CGDisplayPixelsHigh(displayID));
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	const int screen = DefaultScreen(display);

	return MMSignedSizeMake((size_t)DisplayWidth(display, screen),
	                  (size_t)DisplayHeight(display, screen));
#elif defined(IS_WINDOWS)
	return MMSignedSizeMake((size_t)GetSystemMetrics(SM_CXVIRTUALSCREEN),
	                  (size_t)GetSystemMetrics(SM_CYVIRTUALSCREEN));
#endif
}

bool pointVisibleOnMainDisplay(MMPoint point)
{
	MMSignedSize displaySize = getMainDisplaySize();
	return point.x < displaySize.width && point.y < displaySize.height;
}
