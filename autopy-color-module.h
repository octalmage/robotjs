#pragma once
#ifndef AUTOPY_COLOR_MODULE_H
#define AUTOPY_COLOR_MODULE_H

#include <Python.h>

/* Summary: autopy module for converting color formats */
/* Description: This module provides routines for converting between the color
                format (hexadecimal) used by autopy methods and other, more
                readable, formats (e.g., RGB tuples). */
PyMODINIT_FUNC initcolor(void);

#endif /* AUTOPY_COLOR_MODULE_H */
