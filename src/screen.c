#include "screen.h"
#include "os.h"
#include <stdlib.h>

#if defined(IS_MACOSX)
	#include <ApplicationServices/ApplicationServices.h>
#elif defined(USE_X11)
	#include <X11/Xlib.h>
	#include "xdisplay.h"
#elif defined(IS_WINDOWS)
typedef struct _MMMonitorEnumContext {
	MMDisplay *displayList;
	size_t count;
	size_t capacity;
} MMMonitorEnumContext;

static BOOL CALLBACK collectMonitor(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM data)
{
	MMMonitorEnumContext *context = (MMMonitorEnumContext *)data;
	MONITORINFO monitorInfo;
	MMDisplay *nextList = NULL;

	(void)hdc;
	(void)rect;

	if (context == NULL) {
		return FALSE;
	}

	monitorInfo.cbSize = sizeof(monitorInfo);
	if (!GetMonitorInfo(monitor, &monitorInfo)) {
		return TRUE;
	}

	if (context->count == context->capacity) {
		size_t nextCapacity = context->capacity == 0 ? 4 : context->capacity * 2;
		nextList = realloc(context->displayList, nextCapacity * sizeof(MMDisplay));
		if (nextList == NULL) {
			return FALSE;
		}

		context->displayList = nextList;
		context->capacity = nextCapacity;
	}

	context->displayList[context->count].id = (uint32_t)(context->count + 1);
	context->displayList[context->count].x = (int32_t)monitorInfo.rcMonitor.left;
	context->displayList[context->count].y = (int32_t)monitorInfo.rcMonitor.top;
	context->displayList[context->count].width =
		(size_t)(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
	context->displayList[context->count].height =
		(size_t)(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);
	context->displayList[context->count].isMain =
		(monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
	++context->count;

	return TRUE;
}
#endif

MMSize getMainDisplaySize(void)
{
#if defined(IS_MACOSX)
	CGDirectDisplayID displayID = CGMainDisplayID();
	return MMSizeMake(CGDisplayPixelsWide(displayID),
	                  CGDisplayPixelsHigh(displayID));
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	int screen;

	if (display == NULL) {
		return MMSizeMake(0, 0);
	}

	screen = DefaultScreen(display);
	return MMSizeMake((size_t)DisplayWidth(display, screen),
	                  (size_t)DisplayHeight(display, screen));
#elif defined(IS_WINDOWS)
	return MMSizeMake((size_t)GetSystemMetrics(SM_CXSCREEN),
	                  (size_t)GetSystemMetrics(SM_CYSCREEN));
#endif
}

MMDisplay *getDisplayList(size_t *count)
{
	if (count == NULL) {
		return NULL;
	}

#if defined(IS_MACOSX)
	uint32_t displayCount = 0;
	const uint32_t maxDisplays = 32;
	CGDirectDisplayID mainDisplayID = CGMainDisplayID();
	CGDirectDisplayID displayIDs[32];
	MMDisplay *displayList = NULL;

	*count = 0;

	if (CGGetActiveDisplayList(maxDisplays, displayIDs, &displayCount) != kCGErrorSuccess ||
	    displayCount == 0) {
		displayCount = 1;
		displayList = calloc(displayCount, sizeof(MMDisplay));
		if (displayList == NULL) {
			return NULL;
		}

		CGRect bounds = CGDisplayBounds(mainDisplayID);
		displayList[0].id = mainDisplayID;
		displayList[0].x = (int32_t)bounds.origin.x;
		displayList[0].y = (int32_t)bounds.origin.y;
		displayList[0].width = (size_t)bounds.size.width;
		displayList[0].height = (size_t)bounds.size.height;
		displayList[0].isMain = true;
		*count = 1;
		return displayList;
	}

	displayList = calloc(displayCount, sizeof(MMDisplay));
	if (displayList == NULL) {
		return NULL;
	}

	for (uint32_t index = 0; index < displayCount; ++index) {
		CGRect bounds = CGDisplayBounds(displayIDs[index]);
		displayList[index].id = displayIDs[index];
		displayList[index].x = (int32_t)bounds.origin.x;
		displayList[index].y = (int32_t)bounds.origin.y;
		displayList[index].width = (size_t)bounds.size.width;
		displayList[index].height = (size_t)bounds.size.height;
		displayList[index].isMain = displayIDs[index] == mainDisplayID;
	}

	*count = displayCount;
	return displayList;
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	MMDisplay *displayList = NULL;
	int screen;

	if (display == NULL) {
		*count = 0;
		return NULL;
	}

	screen = DefaultScreen(display);

	displayList = calloc(1, sizeof(MMDisplay));
	if (displayList == NULL) {
		*count = 0;
		return NULL;
	}

	displayList[0].id = (uint32_t)screen;
	displayList[0].x = 0;
	displayList[0].y = 0;
	displayList[0].width = (size_t)DisplayWidth(display, screen);
	displayList[0].height = (size_t)DisplayHeight(display, screen);
	displayList[0].isMain = true;
	*count = 1;
	return displayList;
#elif defined(IS_WINDOWS)
	MMDisplay *displayList = calloc(1, sizeof(MMDisplay));
	MMMonitorEnumContext context;

	context.displayList = NULL;
	context.count = 0;
	context.capacity = 0;

	if (EnumDisplayMonitors(NULL, NULL, collectMonitor, (LPARAM)&context)) {
		if (context.count > 0) {
			*count = context.count;
			return context.displayList;
		}
	}

	free(context.displayList);
	if (displayList == NULL) {
		*count = 0;
		return NULL;
	}

	displayList[0].id = 1;
	displayList[0].x = 0;
	displayList[0].y = 0;
	displayList[0].width = (size_t)GetSystemMetrics(SM_CXSCREEN);
	displayList[0].height = (size_t)GetSystemMetrics(SM_CYSCREEN);
	displayList[0].isMain = true;
	*count = 1;
	return displayList;
#endif
}

void destroyDisplayList(MMDisplay *displayList)
{
	free(displayList);
}

bool pointVisibleOnMainDisplay(MMPoint point)
{
	MMSize displaySize = getMainDisplaySize();
	return point.x < displaySize.width && point.y < displaySize.height;
}
