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

#define MMMouseDragToCGEventType(button) \
	((button) == LEFT_BUTTON ? kCGEventLeftMouseDragged \
	                         : ((button) == RIGHT_BUTTON ? kCGEventRightMouseDragged \
	                                                     : kCGEventOtherMouseDragged))

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

#if defined(IS_MACOSX)
/**
 * Calculate the delta for a mouse move and add them to the event.
 * @param event The mouse move event (by ref).
 * @param point The new mouse x and y.
 */
void calculateDeltas(CGEventRef *event, MMPoint point)
{
	/**
	 * The next few lines are a workaround for games not detecting mouse moves.
	 * See this issue for more information:
	 * https://github.com/octalmage/robotjs/issues/159
	 */
	CGEventRef get = CGEventCreate(NULL);
	CGPoint mouse = CGEventGetLocation(get);

	// Calculate the deltas.
	int64_t deltaX = point.x - mouse.x;
	int64_t deltaY = point.y - mouse.y;

	CGEventSetIntegerValueField(*event, kCGMouseEventDeltaX, deltaX);
	CGEventSetIntegerValueField(*event, kCGMouseEventDeltaY, deltaY);

	CFRelease(get);
}
#endif


/**
 * Move the mouse to a specific point.
 * @param point The coordinates to move the mouse to (x, y).
 */
void moveMouse(MMPoint point)
{
#if defined(IS_MACOSX)
	CGEventRef move = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved,
	                                          CGPointFromMMPoint(point),
	                                          kCGMouseButtonLeft);

	calculateDeltas(&move, point);

	CGEventPost(kCGSessionEventTap, move);
	CFRelease(move);
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	XWarpPointer(display, None, DefaultRootWindow(display),
	             0, 0, 0, 0, point.x, point.y);
	XFlush(display);
#elif defined(IS_WINDOWS)
	//Mouse motion is now done using SendInput with MOUSEINPUT. We use Absolute mouse positioning
	#define MOUSE_COORD_TO_ABS(coord, width_or_height) (((65536 * coord) / width_or_height) + (coord < 0 ? -1 : 1))
	point.x = MOUSE_COORD_TO_ABS(point.x, GetSystemMetrics(SM_CXSCREEN));
	point.y = MOUSE_COORD_TO_ABS(point.y, GetSystemMetrics(SM_CYSCREEN));
	INPUT mouseInput;
	mouseInput.type = INPUT_MOUSE;
	mouseInput.mi.dx = point.x;
	mouseInput.mi.dy = point.y;
	mouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
	mouseInput.mi.time = 0; //System will provide the timestamp
	mouseInput.mi.dwExtraInfo = 0;
	mouseInput.mi.mouseData = 0;
	SendInput(1, &mouseInput, sizeof(mouseInput));
	
#endif
}

void dragMouse(MMPoint point, const MMMouseButton button)
{
#if defined(IS_MACOSX)
	const CGEventType dragType = MMMouseDragToCGEventType(button);
	CGEventRef drag = CGEventCreateMouseEvent(NULL, dragType,
	                                                CGPointFromMMPoint(point),
	                                                (CGMouseButton)button);
	calculateDeltas(&drag, point);

	CGEventPost(kCGSessionEventTap, drag);
	CFRelease(drag);
#else
	moveMouse(point);
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

/**
 * Press down a button, or release it.
 * @param down   True for down, false for up.
 * @param button The button to press down or release.
 */
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
 * Special function for sending double clicks, needed for Mac OS X.
 * @param button Button to click.
 */
void doubleClick(MMMouseButton button)
{
	
#if defined(IS_MACOSX)

	/* Double click for Mac. */
	const CGPoint currentPos = CGPointFromMMPoint(getMousePos());
	const CGEventType mouseTypeDown = MMMouseToCGEventType(true, button);
	const CGEventType mouseTypeUP = MMMouseToCGEventType(false, button);

	CGEventRef event = CGEventCreateMouseEvent(NULL, mouseTypeDown, currentPos, kCGMouseButtonLeft);  
	
	/* Set event to double click. */						
	CGEventSetIntegerValueField(event, kCGMouseEventClickState, 2);
											
	CGEventPost(kCGHIDEventTap, event);  
																
	CGEventSetType(event, mouseTypeUP);  
	CGEventPost(kCGHIDEventTap, event);  

	CFRelease(event); 

#else

	/* Double click for everything else. */
	clickMouse(button);
	microsleep(200);
	clickMouse(button);
	
#endif
}

/**
 * Function used to scroll the screen in the required direction.
 * This uses the magnitude to scroll the required amount in the direction. 
 * TODO Requires further fine tuning based on the requirements.
 */
void scrollMouse(int scrollMagnitude, MMMouseWheelDirection scrollDirection)
{
	#if defined(IS_WINDOWS)
		// Fix for #97 https://github.com/octalmage/robotjs/issues/97,
		// C89 needs variables declared on top of functions (mouseScrollInput)
		INPUT mouseScrollInput;
	#endif

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
