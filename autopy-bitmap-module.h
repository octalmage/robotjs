#pragma once
#ifndef AUTOPY_BITMAP_MODULE_H
#define AUTOPY_BITMAP_MODULE_H

#include <Python.h>

/* Summary: autopy module for working with bitmaps */
/* Description: This module defines the class `Bitmap` for accessing
                bitmaps and searching for bitmaps on-screen.

                It also defines functions for taking screenshots of the screen. */
PyMODINIT_FUNC initbitmap(void);

#endif /* AUTOPY_BITMAP_MODULE_H */
