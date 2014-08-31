#include "autopy-bitmap-module.h"
#include "py-bitmap-class.h"
#include "screen.h"
#include "screengrab.h"
#include "py-convenience.h"

/* Syntax: capture_screen(rect=None) => Bitmap object */
/* Arguments: |rect| => ((|x|, |y|), (|width|, |height|)) rect of ints */
/* Description: Returns a screengrab of the given portion of the main display,
                or the entire display if |rect| is None. */
/* Raises: |ValueError| if the rect is out of bounds,
           |OSError| if the screengrab was unsuccessful. */
static PyObject *bitmap_capture_screen(PyObject *self, PyObject *args);

static PyMethodDef BitmapMethods[] = {
	{"capture_screen", bitmap_capture_screen, METH_NOARGS | METH_O,
	 "capture_screen(rect=None) -> Bitmap object\n"
	 "Returns a screengrab of the given portion of the main display,\n"
	 "or the entire display if rect is None."},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

PyMODINIT_FUNC initbitmap(void)
{
	PyObject *mod = Py_InitModule3("bitmap", BitmapMethods,
	                               "autopy module for working with bitmaps");
	if (mod == NULL) return; /* Error */

	/* Instantiate new "Bitmap" class so that it is available in the module. */
	if (Py_AddClassToModule(mod, &Bitmap_Type) < 0) {
		return; /* Error */
	}
}

static PyObject *bitmap_capture_screen(PyObject *self, PyObject *arg)
{
	MMRect rect;
	MMBitmapRef bitmap = NULL;
	MMSize displaySize = getMainDisplaySize();

	if (arg == NULL || arg == Py_None) {
		rect = MMRectMake(0, 0, displaySize.width, displaySize.height);
	} else {
		if (!PyArg_ParseTuple(arg, "(kk)(kk)", &rect.origin.x,
		                                       &rect.origin.y,
		                                       &rect.size.width,
		                                       &rect.size.height)) {
			PyErr_SetString(PyExc_TypeError, "Argument is not a rect");
			return NULL;
		}

		if (rect.origin.x >= displaySize.width ||
		    rect.origin.y >= displaySize.height ||
		    rect.origin.x + rect.size.width > displaySize.width ||
		    rect.origin.y + rect.size.height > displaySize.height) {
			PyErr_SetString(PyExc_ValueError, "Rect out of bounds");
			return NULL;
		}
	}

	bitmap = copyMMBitmapFromDisplayInRect(rect);
	if (bitmap == NULL || bitmap->imageBuffer == NULL) {
		PyErr_SetString(PyExc_OSError,
		                "Could not copy RGB data from display");
		return NULL;
	}

	return BitmapObject_FromMMBitmap(bitmap);
}
