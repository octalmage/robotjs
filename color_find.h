#pragma once
#ifndef COLOR_FIND_H
#define COLOR_FIND_H

#include "MMBitmap.h"
#include "MMPointArray.h"

/* Convenience wrapper around findColorInRect(), where |rect| is the bounds of
 * the image. */
#define findColorInImage(image, color, pointPtr, tolerance) \
	findColorInRect(image, color, pointPtr, MMBitmapGetBounds(image), tolerance)

/* Attempt to find a pixel with the given color in |image| inside |rect|.
 * Returns 0 on success, non-zero on failure. If the color was found and
 * |point| is not NULL, it will be initialized to the (x, y) coordinates the
 * RGB color.
 *
 * |tolerance| should be in the range 0.0f - 1.0f, denoting how closely the
 * colors need to match, with 0 being exact and 1 being any. */
int findColorInRect(MMBitmapRef image, MMRGBHex color, MMPoint *point,
                    MMRect rect, float tolerance);

/* Convenience wrapper around findAllRGBInRect(), where |rect| is the bounds of
 * the image. */
#define findAllColorInImage(image, color, tolerance) \
	findAllColorInRect(image, color, MMBitmapGetBounds(image), tolerance)

/* Returns MMPointArray of all pixels of given color in |image| inside of
 * |rect|. Note that an array is returned regardless of whether the color was
 * found; check array->count to see if it actually was.
 *
 * Responsibility for freeing the MMPointArray with destroyMMPointArray() is
 * given to the caller.
 *
 * |tolerance| should be in the range 0.0f - 1.0f, denoting how closely the
 * colors need to match, with 0 being exact and 1 being any. */
MMPointArrayRef findAllColorInRect(MMBitmapRef image, MMRGBHex color,
                                   MMRect rect, float tolerance);

/* Convenience wrapper around countOfColorsInRect, where |rect| is the bounds
 * of the image. */
#define countOfColorsInImage(image, color, tolerance) \
	countOfColorsInRect(image, color, MMBitmapGetBounds(image), tolerance)

/* Returns the count of the given color in |rect| inside of |image|. */
size_t countOfColorsInRect(MMBitmapRef image, MMRGBHex color, MMRect rect,
                           float tolerance);

#endif /* COLOR_FIND_H */
