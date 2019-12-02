#pragma once
#ifndef MOUSE_H
#define MOUSE_H

#include "os.h"
#include "types.h"

#if defined(_MSC_VER)
	#include "ms_stdbool.h"
#else
	#include <stdbool.h>
#endif
#ifdef __cplusplus
extern "C"
{
#endif
#if defined(IS_MACOSX)

	#include <ApplicationServices/ApplicationServices.h>

	typedef enum  {
		LEFT_BUTTON = kCGMouseButtonLeft,
		RIGHT_BUTTON = kCGMouseButtonRight,
		CENTER_BUTTON = kCGMouseButtonCenter
	} MMMouseButton;

#elif defined(USE_X11)

	enum _MMMouseButton {
		LEFT_BUTTON = 1,
		CENTER_BUTTON = 2,
		RIGHT_BUTTON = 3
	};
	typedef unsigned int MMMouseButton;

#elif defined(IS_WINDOWS)

	enum _MMMouseButton {
		LEFT_BUTTON = 1,
		CENTER_BUTTON = 2,
		RIGHT_BUTTON = 3
	};
	typedef unsigned int MMMouseButton;

#else
	#error "No mouse button constants set for platform"
#endif

#define MMMouseButtonIsValid(button) \
	(button == LEFT_BUTTON || button == RIGHT_BUTTON || \
	 button == CENTER_BUTTON)

enum __MMMouseWheelDirection
{
	DIRECTION_DOWN 	= -1,
	DIRECTION_UP	= 1
};
typedef int MMMouseWheelDirection;

/* Updates information about current virtual screen size and coordinates
 * in Windows
 * It is up to the caller to ensure that this called before mouse moving
*/
void updateScreenMetrics();

/* Immediately moves the mouse to the given point on-screen.
 * It is up to the caller to ensure that this point is within the
 * screen boundaries. */
void moveMouse(MMSignedPoint point);

/* Like moveMouse, moves the mouse to the given point on-screen, but marks
 * the event as the mouse being dragged on platforms where it is supported.
 * It is up to the caller to ensure that this point is within the screen
 * boundaries. */
void dragMouse(MMSignedPoint point, const MMMouseButton button);

/* Smoothly moves the mouse from the current position to the given point.
 * deadbeef_srand() should be called before using this function.
 *
 * Returns false if unsuccessful (i.e. a point was hit that is outside of the
 * screen boundaries), or true if successful. */
bool smoothlyMoveMouse(MMPoint point,double speed);

/* Returns the coordinates of the mouse on the current screen. */
MMPoint getMousePos(void);

/* Holds down or releases the mouse with the given button in the current
 * position. */
void toggleMouse(bool down, MMMouseButton button);

/* Clicks the mouse with the given button in the current position. */
void clickMouse(MMMouseButton button);

/* Double clicks the mouse with the given button. */
void doubleClick(MMMouseButton button);

/* Scrolls the mouse in the stated direction.
 * TODO: Add a smoothly scroll mouse next. */
void scrollMouse(int x, int y);

#endif /* MOUSE_H */

#ifdef __cplusplus
}
#endif
