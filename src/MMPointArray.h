#pragma once
#ifndef MMARRAY_H
#define MMARRAY_H

#include "types.h"

struct _MMPointArray {
	MMPoint *array; /* Pointer to actual data. */
	size_t count;   /* Number of elements in array. */
	size_t _allocedCount; /* Private; do not use outside of MMPointArray.c. */
};

typedef struct _MMPointArray MMPointArray;
typedef MMPointArray *MMPointArrayRef;

/* Creates array of an initial size (the maximum size is still limitless).
 * This follows the "Create" Rule; i.e., responsibility for "destroying" the
 * array is given to the caller. */
MMPointArrayRef createMMPointArray(size_t initialCount);

/* Frees memory occupied by |pointArray|. Does not accept NULL. */
void destroyMMPointArray(MMPointArrayRef pointArray);

/* Appends a point to an array, increasing the internal size if necessary. */
void MMPointArrayAppendPoint(MMPointArrayRef pointArray, MMPoint point);

/* Retrieve point from array. */
#define MMPointArrayGetItem(a, i) ((a)->array)[i]

/* Set point in array. */
#define MMPointArraySetItem(a, i, item) ((a)->array[i] = item)

#endif /* MMARRAY_H */
