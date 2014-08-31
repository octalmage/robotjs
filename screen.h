#pragma once
#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

#if defined(_MSC_VER)
	#include "ms_stdbool.h"
#else
	#include <stdbool.h>
#endif

/* Returns the size of the main display. */
MMSize getMainDisplaySize(void);

/* Convenience function that returns whether the given point is in the bounds
 * of the main screen. */
bool pointVisibleOnMainDisplay(MMPoint point);

#endif /* SCREEN_H */
