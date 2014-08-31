#include "color_find.h"
#include "screen.h"
#include <stdlib.h>

/* Abstracted, general function to avoid repeated code. */
static int findColorInRectAt(MMBitmapRef image, MMRGBHex color, MMPoint *point,
                             MMRect rect, float tolerance, MMPoint startPoint)
{
	MMPoint scan = startPoint;
	if (!MMBitmapRectInBounds(image, rect)) return -1;

	for (; scan.y < rect.size.height; ++scan.y) {
		for (; scan.x < rect.size.width; ++scan.x) {
			MMRGBHex found = MMRGBHexAtPoint(image, scan.x, scan.y);
			if (MMRGBHexSimilarToColor(color, found, tolerance)) {
				if (point != NULL) *point = scan;
				return 0;
			}
		}
		scan.x = rect.origin.x;
	}

	return -1;
}

int findColorInRect(MMBitmapRef image, MMRGBHex color,
                    MMPoint *point, MMRect rect, float tolerance)
{
	return findColorInRectAt(image, color, point, rect, tolerance, rect.origin);
}

MMPointArrayRef findAllColorInRect(MMBitmapRef image, MMRGBHex color,
                                   MMRect rect, float tolerance)
{
	MMPointArrayRef pointArray = createMMPointArray(0);
	MMPoint point = MMPointZero;

	while (findColorInRectAt(image, color, &point, rect, tolerance, point) == 0) {
		MMPointArrayAppendPoint(pointArray, point);
		ITER_NEXT_POINT(point, rect.size.width, rect.origin.x);
	}

	return pointArray;
}

size_t countOfColorsInRect(MMBitmapRef image, MMRGBHex color, MMRect rect,
                           float tolerance)
{
	size_t count = 0;
	MMPoint point = MMPointZero;

	while (findColorInRectAt(image, color, &point, rect, tolerance, point) == 0) {
		ITER_NEXT_POINT(point, rect.size.width, rect.origin.x);
		++count;
	}

	return count;
}
