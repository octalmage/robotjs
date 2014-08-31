#pragma once
#ifndef BITMAP_H
#define BITMAP_H

#include "types.h"
#include "MMBitmap.h"
#include "MMPointArray.h"

/* Convenience wrapper around findBitmapInRect(), where |rect| is the bounds
 * of |haystack|. */
#define findBitmapInBitmap(needle, haystack, pointPtr, tol) \
	findBitmapInRect(needle, haystack, pointPtr, MMBitmapGetBounds(haystack), tol)

/* Returns 0 and sets |point| to the origin of |needle| in |haystack| if
 * |needle| was found in |haystack| inside of |rect|, or returns -1 if not.
 *
 * |tolerance| should be in the range 0.0f - 1.0f, denoting how closely the
 * colors in the bitmaps need to match, with 0 being exact and 1 being any.
 */
int findBitmapInRect(MMBitmapRef needle, MMBitmapRef haystack,
                     MMPoint *point, MMRect rect, float tolerance);

/* Convenience wrapper around findAllBitmapInRect(), where |rect| is the bounds
 * of |haystack|. */
#define findAllBitmapInBitmap(needle, haystack, tolerance) \
	findAllBitmapInRect(needle, haystack, \
			            MMBitmapGetBounds(haystack), tolerance)

/* Returns MMPointArray of all occurrences of |needle| in |haystack| inside of
 * |rect|. Note that an is returned regardless of whether |needle| was found;
 * check array->count to see if it actually was.
 *
 * |tolerance| should be in the range 0.0f - 1.0f, denoting how closely the
 * colors in the bitmaps need to match, with 0 being exact and 1 being any.
 *
 * Responsibility for freeing the MMPointArray with destroyMMPointArray() is
 * given to the caller.
 */
MMPointArrayRef findAllBitmapInRect(MMBitmapRef needle, MMBitmapRef haystack,
                                    MMRect rect, float tolerance);

/* Convenience wrapper around countOfBitmapInRect(), where |rect| is the bounds
 * of |haystack|. */
#define countOfBitmapInBitmap(needle, haystack, tolerance) \
	countOfBitmapInRect(needle, haystack, MMBitmapGetBounds(haystack), tolerance)

/* Returns the number of occurences of |needle| in |haystack| inside
 * of |rect|. */
size_t countOfBitmapInRect(MMBitmapRef needle, MMBitmapRef haystack,
                           MMRect rect, float tolerance);

#endif /* BITMAP_H */
