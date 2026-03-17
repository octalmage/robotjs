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

struct _MMDisplay {
	uint32_t id;
	int32_t x;
	int32_t y;
	size_t width;
	size_t height;
	bool isMain;
};

typedef struct _MMDisplay MMDisplay;

/* Returns the size of the main display. */
MMSize getMainDisplaySize(void);

/* Returns the active display list. Caller must free with destroyDisplayList(). */
MMDisplay *getDisplayList(size_t *count);

/* Frees a display list returned by getDisplayList(). */
void destroyDisplayList(MMDisplay *displayList);

/* Convenience function that returns whether the given point is in the bounds
 * of the main screen. */
bool pointVisibleOnMainDisplay(MMPoint point);

#ifdef __cplusplus
}
#endif

#endif /* SCREEN_H */
