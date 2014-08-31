#pragma once
#ifndef SCREENGRAB_H
#define SCREENGRAB_H

#include "types.h"
#include "MMBitmap.h"

/* Returns a raw bitmap of screengrab of the display (to be destroyed()'d by
 * caller), or NULL on error. */
MMBitmapRef copyMMBitmapFromDisplayInRect(MMRect rect);

#endif /* SCREENGRAB_H */
