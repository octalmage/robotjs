#include "mouse.h"
#include "screen.h"
#include "deadbeef_rand.h"
#include "microsleep.h"

#include <math.h> /* For floor() */

#if defined(IS_MACOSX)
	#include <ApplicationServices/ApplicationServices.h>
#elif defined(USE_X11)
	#include <X11/Xlib.h>
	#include <X11/extensions/XTest.h>
	#include <stdlib.h>
	#include "xdisplay.h"
#endif

#if !defined(M_SQRT2)
	#define M_SQRT2 1.4142135623730950488016887 /* Fix for MSVC. */
#endif

void moveMouse(MMPoint point)
{
#if defined(IS_MACOSX)
	CGEventRef move = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved,
	                                          CGPointFromMMPoint(point),
	                                          0);
	CGEventPost(kCGSessionEventTap, move);
	CFRelease(move);
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	XWarpPointer(display, None, DefaultRootWindow(display),
	             0, 0, 0, 0, point.x, point.y);
	XFlush(display);
#elif defined(IS_WINDOWS)
	point.x *= 0xFFFF / GetSystemMetrics(SM_CXSCREEN);
	point.y *= 0xFFFF / GetSystemMetrics(SM_CYSCREEN);
	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE,
	            (DWORD)point.x, (DWORD)point.y, 0, 0);
#endif
}

MMPoint getMousePos()
{
#if defined(IS_MACOSX)
	CGEventRef event = CGEventCreate(NULL);
	CGPoint point = CGEventGetLocation(event);
	CFRelease(event);

	return MMPointFromCGPoint(point);
#elif defined(USE_X11)
	int x, y; /* This is all we care about. Seriously. */
	Window garb1, garb2; /* Why you can't specify NULL as a parameter */
	int garb_x, garb_y;  /* is beyond me. */
	unsigned int more_garbage;

	Display *display = XGetMainDisplay();
	XQueryPointer(display, XDefaultRootWindow(display), &garb1, &garb2,
	              &x, &y, &garb_x, &garb_y, &more_garbage);

	return MMPointMake(x, y);
#elif defined(IS_WINDOWS)
	POINT point;
	GetCursorPos(&point);

	return MMPointFromPOINT(point);
#endif
}

/* Some convenience macros for converting our enums to the system API types. */
#if defined(IS_MACOSX)

#define MMMouseToCGEventType(down, button) \
	(down ? MMMouseDownToCGEventType(button) : MMMouseUpToCGEventType(button))

#define MMMouseDownToCGEventType(button) \
	((button) == (LEFT_BUTTON) ? kCGEventLeftMouseDown \
	                       : ((button) == RIGHT_BUTTON ? kCGEventRightMouseDown \
	                                                   : kCGEventOtherMouseDown))

#define MMMouseUpToCGEventType(button) \
	((button) == LEFT_BUTTON ? kCGEventLeftMouseUp \
	                         : ((button) == RIGHT_BUTTON ? kCGEventRightMouseUp \
	                                                     : kCGEventOtherMouseUp))

#elif defined(IS_WINDOWS)

#define MMMouseToMEventF(down, button) \
	(down ? MMMouseDownToMEventF(button) : MMMouseUpToMEventF(button))

#define MMMouseUpToMEventF(button) \
	((button) == LEFT_BUTTON ? MOUSEEVENTF_LEFTUP \
	                         : ((button) == RIGHT_BUTTON ? MOUSEEVENTF_RIGHTUP \
	                                                     : MOUSEEVENTF_MIDDLEUP))

#define MMMouseDownToMEventF(button) \
	((button) == LEFT_BUTTON ? MOUSEEVENTF_LEFTDOWN \
	                         : ((button) == RIGHT_BUTTON ? MOUSEEVENTF_RIGHTDOWN \
	                                                     : MOUSEEVENTF_MIDDLEDOWN))

#endif

void toggleMouse(bool down, MMMouseButton button)
{
#if defined(IS_MACOSX)
	const CGPoint currentPos = CGPointFromMMPoint(getMousePos());
	const CGEventType mouseType = MMMouseToCGEventType(down, button);
	CGEventRef event = CGEventCreateMouseEvent(NULL,
	                                           mouseType,
	                                           currentPos,
	                                           (CGMouseButton)button);
	CGEventPost(kCGSessionEventTap, event);
	CFRelease(event);
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	XTestFakeButtonEvent(display, button, down ? True : False, CurrentTime);
	XFlush(display);
#elif defined(IS_WINDOWS)
	mouse_event(MMMouseToMEventF(down, button), 0, 0, 0, 0);
#endif
}

void clickMouse(MMMouseButton button)
{
	toggleMouse(true, button);
	toggleMouse(false, button);
}

/**
 * Function used to scroll the screen in the required direction.
 * This uses the magnitude to scroll the required amount in the direction. 
 * TODO Requires further fine tuning based on the requirements.
 */
void scrollMouse(int scrollMagnitude, MMMouseWheelDirection scrollDirection)	
{
	/* Direction should only be considered based on the scrollDirection. This
	 * Should not interfere. */
	int cleanScrollMagnitude = abs(scrollMagnitude);
	if (!(scrollDirection == DIRECTION_UP || scrollDirection == DIRECTION_DOWN))
	{
		return;
	}
	
	/* Set up the OS specific solution */
	#if defined(__APPLE__)
	
		CGWheelCount wheel = 1;
		CGEventRef event;
	
		/* Make scroll magnitude negative if we're scrolling down. */
		cleanScrollMagnitude = cleanScrollMagnitude * scrollDirection;
		
		event = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, wheel, cleanScrollMagnitude, 0);
		CGEventPost(kCGHIDEventTap, event);
		
	#elif defined(USE_X11)

		int x;
		int dir = 4; /* Button 4 is up, 5 is down. */
		Display *display = XGetMainDisplay();
		
		if (scrollDirection == DIRECTION_DOWN)
		{
			dir = 5;
		}
	
		for (x = 0; x < cleanScrollMagnitude; x++)
		{
			XTestFakeButtonEvent(display, dir, 1, CurrentTime);
			XTestFakeButtonEvent(display, dir, 0, CurrentTime);
		}
		
		XFlush(display);
		
	#elif defined(IS_WINDOWS)
		INPUT mouseScrollInput;
		mouseScrollInput.type = INPUT_MOUSE;
		mouseScrollInput.mi.dx = 0;
		mouseScrollInput.mi.dy = 0;
		mouseScrollInput.mi.dwFlags = MOUSEEVENTF_WHEEL;
		mouseScrollInput.mi.time = 0;
		mouseScrollInput.mi.dwExtraInfo = 0;
		mouseScrollInput.mi.mouseData = WHEEL_DELTA * scrollDirection * cleanScrollMagnitude;
		SendInput(1, &mouseScrollInput, sizeof(mouseScrollInput));
	#endif
}
/*
 * A crude, fast hypot() approximation to get around the fact that hypot() is
 * not a standard ANSI C function.
 *
 * It is not particularly accurate but that does not matter for our use case.
 *
 * Taken from this StackOverflow answer:
 * http://stackoverflow.com/questions/3506404/fast-hypotenuse-algorithm-for-embedded-processor#3507882
 *
 */
static double crude_hypot(double x, double y)
{
	double big = fabs(x); /* max(|x|, |y|) */
	double small = fabs(y); /* min(|x|, |y|) */

	if (big > small) {
		double temp = big;
		big = small;
		small = temp;
	}

	return ((M_SQRT2 - 1.0) * small) + big;
}

bool smoothlyMoveMouse(MMPoint endPoint)
{
	MMPoint pos = getMousePos();
	MMSize screenSize = getMainDisplaySize();
	double velo_x = 0.0, velo_y = 0.0;
	double distance;

	while ((distance = crude_hypot((double)pos.x - endPoint.x,
	                               (double)pos.y - endPoint.y)) > 1.0) {
		double gravity = DEADBEEF_UNIFORM(5.0, 500.0);
		double veloDistance;
		velo_x += (gravity * ((double)endPoint.x - pos.x)) / distance;
		velo_y += (gravity * ((double)endPoint.y - pos.y)) / distance;

		/* Normalize velocity to get a unit vector of length 1. */
		veloDistance = crude_hypot(velo_x, velo_y);
		velo_x /= veloDistance;
		velo_y /= veloDistance;

		pos.x += floor(velo_x + 0.5);
		pos.y += floor(velo_y + 0.5);

		/* Make sure we are in the screen boundaries!
		 * (Strange things will happen if we are not.) */
		if (pos.x >= screenSize.width || pos.y >= screenSize.height) {
			return false;
		}

		moveMouse(pos);

		/* Wait 1 - 3 milliseconds. */
		microsleep(DEADBEEF_UNIFORM(1.0, 3.0));
	}

	return true;
}
