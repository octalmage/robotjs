#include "xdisplay.h"
#include <stdio.h> /* For fputs() */
#include <stdlib.h> /* For atexit() */

static Display *mainDisplay = NULL;
static int registered = 0;
static char *display_name = ":0.0";
static int display_name_changed = 0;

Display *XGetMainDisplay(void)
{
	/* Close the display if display_name has changed */
	if (display_name_changed) {
		XCloseMainDisplay();
	}
	display_name_changed = 0;

	if (mainDisplay == NULL) {
		/* First try the user set display_name */
		mainDisplay = XOpenDisplay(display_name);

		/* Then try using environment variable DISPLAY */
		if (mainDisplay == NULL) {
			mainDisplay = XOpenDisplay(NULL);
		}

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

char *getXDisplay(void)
{
	return display_name;
}

void setXDisplay(char *name) {
	display_name = name;
	display_name_changed = 1;
}
