#pragma once
#ifndef XDISPLAY_H
#define XDISPLAY_H

#include <X11/Xlib.h>

/* Returns the main display, closed either on exit or when closeMainDisplay()
 * is invoked. This removes a bit of the overhead of calling XOpenDisplay() &
 * XCloseDisplay() everytime the main display needs to be used.
 *
 * Note that this is almost certainly not thread safe. */
Display *XGetMainDisplay(void);

/* Closes the main display if it is open, or does nothing if not. */
void XCloseMainDisplay(void);

#endif /* XDISPLAY_H */
