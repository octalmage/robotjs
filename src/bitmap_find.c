#include "bitmap_find.h"
#include "UTHashTable.h"
#include <assert.h>

/* Node to be used in hash table. */
struct shiftNode {
	UTHashNode_HEAD /* Make structure hashable */
	MMRGBHex color; /* Key */
	MMPoint offset; /* Value */
};

/* --- Hash table helper functions --- */

/* Adds hex-color/offset pair to jump table. */
static void addNodeToTable(UTHashTable *table, MMRGBHex color, MMPoint offset);

/* Returns node associated with color in jump table, or NULL if it
 * doesn't exist. */
static struct shiftNode *nodeForColor(UTHashTable *table, MMRGBHex color);

/* Returns nonzero (true) if table has key, or zero (false) if not. */
#define tableHasKey(table, color) (nodeForColor(table, color) != NULL)

/* --- Boyer-Moore helper functions --- */

/* Calculates the first table for use in a Boyer-Moore search algorithm.
 * Table is in the form [colors: shift_values], where colors are those in
 * |needle|, and the shift values are each color's distance from the rightmost
 * offset. All other colors are assumed to have a shift value equal to the
 * length of needle.
 */
static void initBadShiftTable(UTHashTable *jumpTable, MMBitmapRef needle);

/* Frees memory occupied by calling initBadShiftTable().
 * Currently this is just an alias for destroyHashTable(). */
#define destroyBadShiftTable(jumpTable) destroyHashTable(jumpTable)

/* Returns true if |needle| is found in |haystack| at |offset|. */
static int needleAtOffset(MMBitmapRef needle, MMBitmapRef haystack,
                          MMPoint offset, float tolerance);
/* --- --- */

/* An modification of the Boyer-Moore-Horspool Algorithm, only applied to
 * bitmaps and colors instead of strings and characters.
 *
 * TODO: The Boyer-Moore algorithm (with the second jump table) would probably
 * be more efficient, but this was simpler (for now).
 *
 * The jump table (|badShiftTable|) is passed as a parameter to avoid being
 * recalculated each time. It should be a pointer to a UTHashTable init'd with
 * initBadShiftTable().
 *
 * Returns 0 and sets |point| to the starting point of |needle| in |haystack|
 * if |needle| was found in |haystack|, or returns -1 if not. */
static int findBitmapInRectAt(MMBitmapRef needle,
                                MMBitmapRef haystack,
                                MMPoint *point,
                                MMRect rect,
                                float tolerance,
                                MMPoint startPoint,
                                UTHashTable *badShiftTable)
{
	const size_t scanHeight = rect.size.height - needle->height;
	const size_t scanWidth = rect.size.width - needle->width;
	MMPoint pointOffset = startPoint;
	/* const MMPoint lastPoint = MMPointMake(needle->width - 1, needle->height - 1); */

	/* Sanity check */
	if (needle->height > haystack->height || needle->width > haystack->width ||
	    !MMBitmapRectInBounds(haystack, rect)) {
		return -1;
	}

	assert(point != NULL);
	assert(needle != NULL);
	assert(needle->height > 0 && needle->width > 0);
	assert(haystack != NULL);
	assert(haystack->height > 0 && haystack->width > 0);
	assert(badShiftTable != NULL);

	/* Search |haystack|, while |needle| can still be within it. */
	while (pointOffset.y <= scanHeight) {
		/* struct shiftNode *node = NULL;
		MMRGBHex lastColor; */

		while (pointOffset.x <= scanWidth) {
			/* Check offset in |haystack| for |needle|. */
			if (needleAtOffset(needle, haystack, pointOffset, tolerance)) {
				++pointOffset.x;
				++pointOffset.y;
				*point = pointOffset;
				return 0;
			}

			/* Otherwise, calculate next x offset to check. */
			/*
			 * Note that here we are getting the skip value based on the last
			 * color of |needle|, no matter where we didn't match. The
			 * alternative of pretending that the mismatched color was the previous
			 * color is slower in the normal case.
			 */
			/* lastColor = MMRGBHexAtPoint(haystack, pointOffset.x + lastPoint.x,
			                                      pointOffset.y + lastPoint.y); */

			/* TODO: This fails on certain edge cases (issue#7). */
			/* When a color is encountered that does not occur in |needle|, we can
			 * safely skip ahead for the whole length of |needle|.
			 * Otherwise, use the value stored in the jump table. */
			/* node = nodeForColor(badShiftTable, lastColor);
			pointOffset.x += (node == NULL) ? needle->width : (node->offset).x; */

			/* For now, be naive. */
			++pointOffset.x;
		}

		pointOffset.x = rect.origin.x;

		/* lastColor = MMRGBHexAtPoint(haystack, pointOffset.x + lastPoint.x,
		                               pointOffset.y + lastPoint.y);
		node = nodeForColor(badShiftTable, lastColor);
		pointOffset.y += node == NULL ? lastPoint.y : (node->offset).y; */

		/* TODO: The above commented out code fails at certain edge cases, e.g.:
		 * Needle: [B, b
		 *          b, b,
		 *          B, b]
		 * Haystack: [w, w, w, w, w
		 *            w, w, w, w, b
		 *            w, w, w, b, b
		 *            w, w, w, w, b]
		 * The previous algorithm noticed that the first 3 x 3 block had nothing
		 * in common with the image, and thus, after scanning the first row,
		 * skipped three blocks downward to scan the next (which didn't exist,
		 * so the loop ended). However, the needle was hidden IN-BETWEEN this
		 * jump -- skipping was appropriate for scanning the column but not
		 * the row.
		 *
		 * I need to figure out a more optimal solution; temporarily I am just
		 * scanning every single y coordinate, only skipping on x's. This
		 * always works, but is probably not optimal.
		 */
		++pointOffset.y;
	}

	return -1;
}

int findBitmapInRect(MMBitmapRef needle,
		             MMBitmapRef haystack,
                     MMPoint *point,
                     MMRect rect,
                     float tolerance)
{
	UTHashTable badShiftTable;
	int ret;

	initBadShiftTable(&badShiftTable, needle);
	ret = findBitmapInRectAt(needle, haystack, point, rect,
	                         tolerance, MMPointZero, &badShiftTable);
	destroyBadShiftTable(&badShiftTable);
	return ret;
}

MMPointArrayRef findAllBitmapInRect(MMBitmapRef needle, MMBitmapRef haystack,
                                    MMRect rect, float tolerance)
{
	MMPointArrayRef pointArray = createMMPointArray(0);
	MMPoint point = MMPointZero;
	UTHashTable badShiftTable;

	initBadShiftTable(&badShiftTable, needle);
	while (findBitmapInRectAt(needle, haystack, &point, rect,
	                          tolerance, point, &badShiftTable) == 0) {
		const size_t scanWidth = (haystack->width - needle->width) + 1;
		MMPointArrayAppendPoint(pointArray, point);
		ITER_NEXT_POINT(point, scanWidth, 0);
	}
	destroyBadShiftTable(&badShiftTable);

	return pointArray;
}

size_t countOfBitmapInRect(MMBitmapRef needle, MMBitmapRef haystack,
                           MMRect rect, float tolerance)
{
	size_t count = 0;
	MMPoint point = MMPointZero;
	UTHashTable badShiftTable;

	initBadShiftTable(&badShiftTable, needle);
	while (findBitmapInRectAt(needle, haystack, &point, rect,
	                          tolerance, point, &badShiftTable) == 0) {
		const size_t scanWidth = (haystack->width - needle->width) + 1;
		++count;
		ITER_NEXT_POINT(point, scanWidth, 0);
	}
	destroyBadShiftTable(&badShiftTable);

	return count;
}

/* --- Boyer-Moore helper functions --- */

static void initBadShiftTable(UTHashTable *jumpTable, MMBitmapRef needle)
{
	const MMPoint lastPoint = MMPointMake(needle->width - 1, needle->height - 1);
	const size_t maxColors = needle->width * needle->height;
	MMPoint scan;

	/* Allocate max size initially to avoid a million calls to malloc(). */
	initHashTable(jumpTable, maxColors, sizeof(struct shiftNode));

	/* Populate jumpTable with analysis of |needle|. */
	for (scan.y = lastPoint.y; ; --scan.y) {
		for (scan.x = lastPoint.x; ; --scan.x) {
			MMRGBHex color = MMRGBHexAtPoint(needle, scan.x, scan.y);
			if (!tableHasKey(jumpTable, color)) {
				addNodeToTable(jumpTable, color,
				               MMPointMake(needle->width - scan.x,
				                           needle->height - scan.y));
			}

			if (scan.x == 0) break; /* Avoid infinite loop from unsigned type. */
		}
		if (scan.y == 0) break;
	}
}

static int needleAtOffset(MMBitmapRef needle, MMBitmapRef haystack,
                          MMPoint offset, float tolerance)
{
	const MMPoint lastPoint = MMPointMake(needle->width - 1, needle->height - 1);
	MMPoint scan;

	/* Note that |needle| is searched backwards, in accordance with the
	 * Boyer-Moore search algorithm. */
	for (scan.y = lastPoint.y; ; --scan.y) {
		for (scan.x = lastPoint.x; ; --scan.x) {
			MMRGBHex ncolor = MMRGBHexAtPoint(needle, scan.x, scan.y);
			MMRGBHex hcolor = MMRGBHexAtPoint(haystack, offset.x + scan.x,
			                                            offset.y + scan.y);
			if (!MMRGBHexSimilarToColor(ncolor, hcolor, tolerance)) return 0;
			if (scan.x == 0) break; /* Avoid infinite loop from unsigned type. */
		}
		if (scan.y == 0) break;
	}

	return 1;
}

/* --- Hash table helper functions --- */

static void addNodeToTable(UTHashTable *table,
                           MMRGBHex hexColor,
                           MMPoint offset)
{
	struct shiftNode *node = getNewNode(table);
	node->color = hexColor;
	node->offset = offset;
	UTHASHTABLE_ADD_INT(table, color, node, struct shiftNode);
}

static struct shiftNode *nodeForColor(UTHashTable *table,
                                      MMRGBHex color)
{
	struct shiftNode *uttable = table->uttable;
	struct shiftNode *node;
	HASH_FIND_INT(uttable, &color, node);
	return node;
}
