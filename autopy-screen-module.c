#include "autopy-screen-module.h"
#include "screen.h"
#include "screengrab.h"

/* Syntax: get_size() => tuple (width, height) */
/* Description: Returns a tuple `(width, height)` of the size of the
                main screen. */
static PyObject *screen_get_size(PyObject *self, PyObject *args);

/* Syntax: point_visible(x, y) => Boolean */
/* Arguments: |x| => integer,
              |y| => integer */
/* Description: Returns True if the given point is inside the main
                screen boundaries. */
static PyObject *screen_point_visible(PyObject *self, PyObject *args);

/* Syntax: get_color(x, y) => hexadecimal integer */
/* Arguments: |x| => integer,
              |y| => integer */
/* Description: Returns hexadecimal value describing the RGB color at the
                given point.

                Essentially equivalent to:
                {%
                    rect = ((x, y), (1, 1))
                    bitmap.capture_screen_portion(rect).get_color(0, 0)
                %}
                only more efficient/convenient. */
/* Raises: |ValueError| if the rect is out of bounds,
           |OSError| if the system calls were unsuccessful. */
static PyObject *screen_get_color(PyObject *self, PyObject *args);

static PyMethodDef ScreenMethods[] = {
	{"get_size", screen_get_size, METH_NOARGS,
	 "get_size() -> tuple (width, height)\n"
	 "Returns a tuple (width, height) of the size of the main screen."},
	{"point_visible", screen_point_visible, METH_VARARGS,
	 "point_visible(x, y) -> Boolean\n"
	 "Returns whether given (x, y) coordinate is inside the main screen."},
	{"get_color", screen_get_color, METH_VARARGS,
	 "get_color(x, y) -> integer\n"
	 "Returns hexadecimal value describing the RGB color at the given point."},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

PyMODINIT_FUNC initscreen(void)
{
	Py_InitModule3("screen", ScreenMethods, "autopy module for working with "
	                                        "the screen");
}

static PyObject *screen_get_size(PyObject *self, PyObject *args)
{
	MMSize screenSize = getMainDisplaySize();
	return Py_BuildValue("(kk)", screenSize.width, screenSize.height);
}

static PyObject *screen_point_visible(PyObject *self, PyObject *args)
{
	MMPoint point;
	if (!PyArg_ParseTuple(args, "kk", &point.x, &point.y)) {
		return NULL;
	}

	if (!pointVisibleOnMainDisplay(point)) {
		Py_RETURN_FALSE;
	}

	Py_RETURN_TRUE;
}

static PyObject *screen_get_color(PyObject *self, PyObject *args)
{
	MMPoint point;

	MMBitmapRef bitmap;
	MMRGBHex color;

	if (!PyArg_ParseTuple(args, "kk", &point.x, &point.y)) {
		return NULL;
	}

	if (!pointVisibleOnMainDisplay(point)) {
		PyErr_SetString(PyExc_ValueError, "Point out of bounds");
		return NULL;
	}

	bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(point.x, point.y, 1, 1));
	if (bitmap == NULL || bitmap->imageBuffer == NULL) {
		PyErr_SetString(PyExc_OSError,
		                "Could not copy RGB data from display.");
		return NULL;
	}

	color = MMRGBHexAtPoint(bitmap, 0, 0);
	destroyMMBitmap(bitmap);
	return Py_BuildValue("I", color);
}
