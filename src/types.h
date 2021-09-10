#pragma once
#ifndef TYPES_H
#define TYPES_H

#include "os.h"
#include "inline_keywords.h" /* For H_INLINE */
#include <stddef.h>
#include <stdint.h>

/* Some generic, cross-platform types. */

struct _MMPoint {
	size_t x;
	size_t y;
};

typedef struct _MMPoint MMPoint;


struct _MMSignedPoint {
	int32_t x;
	int32_t y;
};

typedef struct _MMSignedPoint MMSignedPoint;

struct _MMSize {
	size_t width;
	size_t height;
};

typedef struct _MMSize MMSize;

struct _MMSignedSize {
	int32_t width;
	int32_t height;
};

typedef struct _MMSignedSize MMSignedSize;

struct _MMRect {
	MMPoint origin;
	MMSize size;
};

typedef struct _MMRect MMRect;

H_INLINE MMPoint MMPointMake(size_t x, size_t y)
{
	MMPoint point;
	point.x = x;
	point.y = y;
	return point;
}

H_INLINE MMSignedPoint MMSignedPointMake(int32_t x, int32_t y)
{
	MMSignedPoint point;
	point.x = x;
	point.y = y;
	return point;
}

H_INLINE MMSize MMSizeMake(size_t width, size_t height)
{
	MMSize size;
	size.width = width;
	size.height = height;
	return size;
}
H_INLINE MMSignedSize MMSignedSizeMake(int32_t width, int32_t height)
{
	MMSignedSize size;
	size.width = width;
	size.height = height;
	return size;
}

H_INLINE MMRect MMRectMake(size_t x, size_t y, size_t width, size_t height)
{
	MMRect rect;
	rect.origin = MMPointMake(x, y);
	rect.size = MMSizeMake(width, height);
	return rect;
}

#define MMPointZero MMPointMake(0, 0)

#if defined(IS_MACOSX)

#define CGPointFromMMPoint(p) CGPointMake((CGFloat)(p).x, (CGFloat)(p).y)
#define MMPointFromCGPoint(p) MMPointMake((size_t)(p).x, (size_t)(p).y)

#define CGPointFromMMSignedPoint(p) CGPointMake((CGFloat)(p).x, (CGFloat)(p).y)
#define MMSignedPointFromCGPoint(p) MMPointMake((int32_t)(p).x, (int32_t)(p).y)

#elif defined(IS_WINDOWS)

#define MMPointFromPOINT(p) MMPointMake((size_t)p.x, (size_t)p.y)
#define MMSignedPointFromPOINT(p) MMSignedPointMake((int32_t)p.x, (int32_t)p.y)

#endif

#endif /* TYPES_H */
