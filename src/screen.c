#include "screen.h"
#include "os.h"

#if defined(IS_MACOSX)
	#include <ApplicationServices/ApplicationServices.h>
#elif defined(USE_X11)
	#include <X11/Xlib.h>
	#include "xdisplay.h"
#endif

void getAllDisplaySize(uint32_t *_Nullable numDisplays, MMDisplaySize *_Nullable displaySizes)
{
#if defined(IS_MACOSX)

	CGDirectDisplayID displays[10];
	CGDirectDisplayID mainDisplayID = CGMainDisplayID();

	CGGetOnlineDisplayList(10, displays, numDisplays);
	for (uint32_t i = 0; i < *numDisplays; i++)
	{
		CGRect bounds = CGDisplayBounds(displays[i]);
		displaySizes[i] = MMDisplaySizeMake(
			displays[i],
			mainDisplayID == displays[i],
			CGDisplayPixelsWide(displays[i]),
			CGDisplayPixelsHigh(displays[i]),
			MMRectMake(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height)
		);
	}

#endif
}

MMSize getMainDisplaySize(void)
{
#if defined(IS_MACOSX)
	CGDirectDisplayID displayID = CGMainDisplayID();
	return MMSizeMake(CGDisplayPixelsWide(displayID),
	                  CGDisplayPixelsHigh(displayID));
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	const int screen = DefaultScreen(display);

	return MMSizeMake((size_t)DisplayWidth(display, screen),
	                  (size_t)DisplayHeight(display, screen));
#elif defined(IS_WINDOWS)
	return MMSizeMake((size_t)GetSystemMetrics(SM_CXSCREEN),
	                  (size_t)GetSystemMetrics(SM_CYSCREEN));
#endif
}

bool pointVisibleOnMainDisplay(MMPoint point)
{
	MMSize displaySize = getMainDisplaySize();
	return point.x < displaySize.width && point.y < displaySize.height;
}
