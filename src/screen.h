#pragma once
#ifndef SCREEN_H
#define SCREEN_H

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

/* Returns the size of the main display. */
MMSize getMainDisplaySize(void);

/* Returns a list of displayID, with their sizes. */
void getAllDisplaySize(uint32_t *_Nullable numDisplays, MMDisplaySize *_Nullable displaySizes);

/* Convenience function that returns whether the given point is in the bounds
 * of the main screen. */
bool pointVisibleOnMainDisplay(MMPoint point);

#ifdef __cplusplus
}
#endif

#endif /* SCREEN_H */
