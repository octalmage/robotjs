#include "xdisplay.h"
#include <stdio.h> /* For fputs() */
#include <stdlib.h> /* For atexit() */

static Display *mainDisplay = NULL;
static int registered = 0;

Display *XGetMainDisplay(void)
{
	if (mainDisplay == NULL) {
		mainDisplay = XOpenDisplay(NULL);

		if (mainDisplay == NULL) {
			fputs("Could not open main display\n", stderr);
		} else if (!registered) {
			atexit(&XCloseMainDisplay);
			registered = 1;
		}
	}

	return mainDisplay;
}

void XCloseMainDisplay(void)
{
	if (mainDisplay != NULL) {
		XCloseDisplay(mainDisplay);
		mainDisplay = NULL;
	}
}
