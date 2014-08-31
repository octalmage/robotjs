#include "MMPointArray.h"
#include <stdlib.h>

MMPointArrayRef createMMPointArray(size_t initialCount)
{
	MMPointArrayRef pointArray = calloc(1, sizeof(MMPointArray));

	if (initialCount == 0) initialCount = 1;

	pointArray->_allocedCount = initialCount;
	pointArray->array = malloc(pointArray->_allocedCount * sizeof(MMPoint));
	if (pointArray->array == NULL) return NULL;

	return pointArray;
}

void destroyMMPointArray(MMPointArrayRef pointArray)
{
	if (pointArray->array != NULL) {
		free(pointArray->array);
		pointArray->array = NULL;
	}

	free(pointArray);
}

void MMPointArrayAppendPoint(MMPointArrayRef pointArray, MMPoint point)
{
	const size_t newCount = ++(pointArray->count);
	if (pointArray->_allocedCount < newCount) {
		do {
			/* Double size each time to avoid calls to realloc(). */
			pointArray->_allocedCount <<= 1;
		} while (pointArray->_allocedCount < newCount);
		pointArray->array = realloc(pointArray->array,
		                            sizeof(point) *
		                            pointArray->_allocedCount);
	}

	pointArray->array[pointArray->count - 1] = point;
}
