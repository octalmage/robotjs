#pragma once
#ifndef SCREENGRAB_H
#define SCREENGRAB_H

#include "screen.h"
#include "MMBitmap.h"

#ifdef __cplusplus
extern "C" 
{
#endif

/* Returns a raw bitmap of screengrab of the display (to be destroyed()'d by
 * caller), or NULL on error. */
MMBitmapRef copyMMBitmapFromDisplayInRect(MMRect rect);

/* Returns a raw bitmap of screengrab of the given display in display-local
 * coordinates (to be destroyed()'d by caller), or NULL on error. */
MMBitmapRef copyMMBitmapFromDisplayInRectOnDisplay(MMDisplay display, MMRect rect);

#ifdef __cplusplus
}
#endif

#endif /* SCREENGRAB_H */
