#pragma once
#ifndef SCREENGRAB_H
#define SCREENGRAB_H

#include "types.h"
#include "MMBitmap.h"

#ifdef __cplusplus
extern "C" 
{
#endif

/* Returns a raw bitmap of screengrab of the display (to be destroyed()'d by
 * caller), or NULL on error. */
MMBitmapRef copyMMBitmapFromDisplayInRect(int32_t displayID, MMRect rect);

#ifdef __cplusplus
}
#endif

#endif /* SCREENGRAB_H */
