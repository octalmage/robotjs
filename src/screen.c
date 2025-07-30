#include "screen.h"
#include "os.h"

#if defined(IS_MACOSX)
	#include <ApplicationServices/ApplicationServices.h>
#elif defined(USE_X11)
	#include <X11/Xlib.h>
	#include "xdisplay.h"
#endif

MMSignedSize getMainDisplaySize(void)
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
	return MMSignedSizeMake((size_t)GetSystemMetrics(SM_CXVIRTUALSCREEN),
	                  (size_t)GetSystemMetrics(SM_CYVIRTUALSCREEN));
#endif
}

bool pointVisibleOnMainDisplay(MMPoint point)
{
	MMSignedSize displaySize = getMainDisplaySize();
	return point.x < displaySize.width && point.y < displaySize.height;
}
