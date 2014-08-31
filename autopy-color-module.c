#include "autopy-color-module.h"
#include "MMBitmap.h"

/* Syntax: hex_to_rgb(hex) => tuple (r, g, b) */
/* Arguments: |hex| => integer (in range 0x000000 - 0xFFFFFF) */
/* Description: Returns a tuple `(r, g, b)` of the RGB integer values equivalent
                to the given RGB hexadecimal value. |r|, |g|, and |b| are in
                the range 0 - 255. */
static PyObject *color_hex_to_rgb(PyObject *self, PyObject *args);

/* Syntax: rgb_to_hex(r, g, b) => integer (in the range 0x000000 - 0xFFFFFF) */
/* Arguments: |r| => integer (0 - 255),
              |b| => integer (0 - 255),
              |g| => integer (0 - 255) */
/* Description: Returns hexadecimal value of given RGB tuple. |r|, |g|, and |b|
                should be in the range 0 - 255. */
static PyObject *color_rgb_to_hex(PyObject *self, PyObject *args);

static PyMethodDef ColorMethods[] = {
	{"hex_to_rgb", color_hex_to_rgb, METH_VARARGS,
	 "hex_to_rgb(hex) -> tuple (r, g, b)\n"
	 "Returns a tuple (r, g, b) of the RGB integer values equivalent to the\n"
	 "given RGB hex value."},
	{"rgb_to_hex", color_rgb_to_hex, METH_VARARGS,
	 "rgb_to_hex(r, g, b) -> integer\n"
	 "Returns hexadecimal value of given RGB tuple, where |r|, |g|, and\n"
	 "|b| in the range 0 - 255, and the returned value is in the\n"
	 "range 0x000000 - 0xFFFFFF."},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

PyMODINIT_FUNC initcolor(void)
{
	Py_InitModule3("color", ColorMethods,
	               "autopy module for converting between color formats");
}

static PyObject *color_hex_to_rgb(PyObject *self, PyObject *args)
{
	MMRGBHex hex;
	if (!PyArg_ParseTuple(args, "I", &hex)) return NULL;

	return Py_BuildValue("(bbb)", RED_FROM_HEX(hex),
	                              GREEN_FROM_HEX(hex),
	                              BLUE_FROM_HEX(hex));
}

static PyObject *color_rgb_to_hex(PyObject *self, PyObject *args)
{
	MMRGBColor color;
	if (!PyArg_ParseTuple(args, "bbb", &color.red,
	                                   &color.green,
	                                   &color.blue)) {
		return NULL;
	}

	return Py_BuildValue("I", hexFromMMRGB(color));
}
