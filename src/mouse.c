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

// The width of the virtual screen, in pixels.
static int vscreenWidth = -1; // not initialized

// The height of the virtual screen, in pixels.
static int vscreenHeight = -1; // not initialized

// The coordinates for the left side of the virtual screen.
static int vscreenMinX = 0;

// The coordinates for the top of the virtual screen.
static int vscreenMinY = 0;

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
void calculateDeltas(CGEventRef *event, MMSignedPoint point)
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

void updateScreenMetrics()
{
	#if defined(IS_WINDOWS)
		vscreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		vscreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		vscreenMinX = GetSystemMetrics(SM_XVIRTUALSCREEN);
		vscreenMinY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	#endif
}
/**
 * Move the mouse to a specific point.
 * @param point The coordinates to move the mouse to (x, y).
 */
void moveMouse(MMSignedPoint point)
{
#if defined(IS_MACOSX)
	CGEventRef move = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved,
	                                          CGPointFromMMSignedPoint(point),
	                                          kCGMouseButtonLeft);

	calculateDeltas(&move, point);

	CGEventPost(kCGSessionEventTap, move);
	CFRelease(move);
#elif defined(USE_X11)
	Display *display = XGetMainDisplay();
	XWarpPointer(display, None, DefaultRootWindow(display),
	             0, 0, 0, 0, point.x, point.y);
	XSync(display, false);
#elif defined(IS_WINDOWS)

	if(vscreenWidth<0 || vscreenHeight<0)
		updateScreenMetrics();

	//Mouse motion is now done using SendInput with MOUSEINPUT. We use Absolute mouse positioning
	#define MOUSE_COORD_TO_ABS(coord, width_or_height) ((65536 * (coord) / width_or_height) + ((coord) < 0 ? -1 : 1))

	size_t x = MOUSE_COORD_TO_ABS(point.x-vscreenMinX, vscreenWidth);
	size_t y = MOUSE_COORD_TO_ABS(point.y-vscreenMinY, vscreenHeight);

	INPUT mouseInput = {0};
	mouseInput.type = INPUT_MOUSE;
	mouseInput.mi.dx = x;
	mouseInput.mi.dy = y;
	mouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
	mouseInput.mi.time = 0; //System will provide the timestamp

	SendInput(1, &mouseInput, sizeof(mouseInput));
#endif
}

void dragMouse(MMSignedPoint point, const MMMouseButton button)
{
#if defined(IS_MACOSX)
	const CGEventType dragType = MMMouseDragToCGEventType(button);
	CGEventRef drag = CGEventCreateMouseEvent(NULL, dragType,
	                                                CGPointFromMMSignedPoint(point),
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
	XSync(display, false);
#elif defined(IS_WINDOWS)
	INPUT mouseInput;
	mouseInput.type = INPUT_MOUSE;
	mouseInput.mi.dx = 0;
	mouseInput.mi.dy = 0;
	mouseInput.mi.dwFlags = MMMouseToMEventF(down, button);
	mouseInput.mi.time = 0; //System will provide the timestamp
	mouseInput.mi.dwExtraInfo = 0;
	mouseInput.mi.mouseData = 0;
	SendInput(1, &mouseInput, sizeof(mouseInput));
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

void scrollMouse(int x, int y)
{
#if defined(IS_WINDOWS)
	// Fix for #97 https://github.com/octalmage/robotjs/issues/97,
	// C89 needs variables declared on top of functions (mouseScrollInput)
	INPUT mouseScrollInputs[2];
#endif

  /* Direction should only be considered based on the scrollDirection. This
   * Should not interfere. */

  /* Set up the OS specific solution */
#if defined(__APPLE__)

	CGEventRef event;

	event = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitPixel, 2, y, x);
	CGEventPost(kCGHIDEventTap, event);

	CFRelease(event);

#elif defined(USE_X11)

	/*
	X11 Mouse Button Numbering
	1 = left button
	2 = middle button (pressing the scroll wheel)
	3 = right button
	4 = turn scroll wheel up
	5 = turn scroll wheel down
	6 = push scroll wheel left
	7 = push scroll wheel right
	8 = 4th button (aka browser backward button)
	9 = 5th button (aka browser forward button)
	*/
	int ydir = 4; // Button 4 is up, 5 is down.
	int xdir = 6; // Button 6 is left, 7 is right.
	Display *display = XGetMainDisplay();

	if (y < 0){
		ydir = 5;
	}
	if (x < 0){
		xdir = 7;
	}

	int xi;
	int yi;
	for (xi = 0; xi < abs(x); xi++) {
		XTestFakeButtonEvent(display, xdir, 1, CurrentTime);
		XTestFakeButtonEvent(display, xdir, 0, CurrentTime);
	}
	for (yi = 0; yi < abs(y); yi++) {
		XTestFakeButtonEvent(display, ydir, 1, CurrentTime);
		XTestFakeButtonEvent(display, ydir, 0, CurrentTime);
	}

	XSync(display, false);

#elif defined(IS_WINDOWS)

	mouseScrollInputs[0].type = INPUT_MOUSE;
	mouseScrollInputs[0].mi.dx = 0;
	mouseScrollInputs[0].mi.dy = 0;
	mouseScrollInputs[0].mi.dwFlags = MOUSEEVENTF_HWHEEL;
	mouseScrollInputs[0].mi.time = 0;
	mouseScrollInputs[0].mi.dwExtraInfo = 0;
	// Flip x to match other platforms.
	mouseScrollInputs[0].mi.mouseData = -x;

	mouseScrollInputs[1].type = INPUT_MOUSE;
	mouseScrollInputs[1].mi.dx = 0;
	mouseScrollInputs[1].mi.dy = 0;
	mouseScrollInputs[1].mi.dwFlags = MOUSEEVENTF_WHEEL;
	mouseScrollInputs[1].mi.time = 0;
	mouseScrollInputs[1].mi.dwExtraInfo = 0;
	mouseScrollInputs[1].mi.mouseData = y;

	SendInput(2, mouseScrollInputs, sizeof(mouseScrollInputs));
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

bool smoothlyMoveMouse(MMPoint endPoint,double speed)
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

		moveMouse(MMSignedPointMake((int32_t)pos.x, (int32_t)pos.y));

		/* Wait 1 - (speed) milliseconds. */
		microsleep(DEADBEEF_UNIFORM(0.7, speed));
	}

	return true;
}
