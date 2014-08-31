#include "py-bitmap-class.h"
#include "bitmap_find.h"
#include "color_find.h"
#include "screen.h"
#include "io.h"
#include "pasteboard.h"
#include "str_io.h"
#include "py-convenience.h"
#include <assert.h>
#include <stdio.h>

/* -- Bitmap class definition -- */

static void Bitmap_dealloc(BitmapObject *self)
{
	if (self->bitmap != NULL) {
		destroyMMBitmap(self->bitmap);
		self->bitmap = NULL;
	}
	self->ob_type->tp_free((PyObject *)self);
}

/* -- Bitmap Class method declarations -- */

/* Syntax: Bitmap.open(filepath, format) => Bitmap object */
/* Arguments: |filepath| => string,
              |format| => string ("BMP" or "PNG") */
/* Description: Opens image at absolute filepath in the given format.
                The image type is determined from the filename if possible,
                unless |format| is given. */
/* Raises: |IOError| if the image could not be opened. */
static PyObject *Bitmap_open(PyObject *self, PyObject *args);

/* Syntax: Bitmap.from_string(string) => Bitmap object */
/* Arguments: |string| => string */
/* Description: Creates bitmap from string created by `bmp.to_string()`. */
/* Raises: |ValueError| if the given string was invalid. */
static PyObject *Bitmap_from_string(PyObject *self, PyObject *args);

/* Syntax: bmp.get_portion(origin, size) => Bitmap object */
/* Arguments: |origin| => |x|, |y| tuple of ints,
              |size| => |width|, |height| tuple of ints */
/* Description: Returns new bitmap object created from a portion of another. */
/* Raises: |ValueError| if portion was out of bounds,
           |IOError| if portion could not be copied. */
static PyObject *Bitmap_get_portion(BitmapObject *self, PyObject *args);

/* -- Bitmap Instance method declarations -- */

/* Syntax: bmp.point_in_bounds(x, y) => Boolean */
/* Arguments: |x| => integer,
              |y| => integer */
/* Description: Returns True if `x < bmp.width` and `y < bmp.height`, or False
                if not. */
static PyObject *Bitmap_point_in_bounds(BitmapObject *self, PyObject *args);

/* Syntax: bmp.copy_to_pboard() */
/* Description: Copies image to pasteboard.
                Currently only supported on Windows and Mac OS X. */
/* Raises: |OSError| if the image could not be copied. */
static PyObject *Bitmap_copy_to_pboard(BitmapObject *self, PyObject *args);

/* Syntax: bmp.save(filepath, [format]) */
/* Arguments: |filepath| => string,
              |format| => string ("BMP" or "PNG") */
/* Description: Saves image to absolute filepath in the given format.
                The image type is determined from the filename if possible,
                unless |format| is given. Note that if the file already exists,
                it WILL be overwritten. Check yourself first if you do not
                desire this. */
/* Raises: |IOError| if the image could not be saved. */
static PyObject *Bitmap_save(BitmapObject *self, PyObject *args);

/* Syntax: bmp.to_string() => string */
/* Description: Returns compressed, printable string representing bitmap, to be
                used with `Bitmap.from_string()`. */
/* Raises: |IOError| if the string could not be created. */
static PyObject *Bitmap_to_string(BitmapObject *self, PyObject *args);

/* Syntax: bmp.get_color(x, y) => hexadecimal integer */
/* Arguments: |x| => integer,
              |y| => integer */
/* Description: Returns hexadecimal value describing the RGB color at the
                given point. */
/* Raises: |ValueError| if the point out of bounds. */
static PyObject *Bitmap_get_color(BitmapObject *self, PyObject *args);

/* Syntax: bmp.find_color(color, tolerance=0.0, rect=None) => tuple (x, y)
                                                              or None */
/* Arguments: |color| => integer (0x000000 - 0xFFFFFF),
              |tolerance| => float,
              |rect| => ((|x|, |y|), (|width|, |height|)) rect of ints,
                        or None */
/* Description: Attempts to find color inside |rect| in |bmp|; returns
                coordinates if found, or None if not. If |rect| is None,
                the entire image is searched. */
static PyObject *Bitmap_find_color(BitmapObject *self, PyObject *args);

/* Syntax: bmp.find_every_color(color, tolerance=0.0, rect=None) =>
                                       list of tuples [(x, y), ...] */
/* Arguments: |color| => integer (0x000000 - 0xFFFFFF),
              |tolerance| => float,
              |rect| => ((|x|, |y|), (|width|, |height|)) rect of ints,
                        or None */
/* Description: Returns list of all coordinates inside |rect| in |bmp| matching
                |color|. If |rect| is None, the entire image is searched. */
static PyObject *Bitmap_find_every_color(BitmapObject *self, PyObject *args);

/* Syntax: bmp.count_of_color(color, tolerance=0.0, rect=None) => integer */
/* Arguments: |color| => integer (0x000000 - 0xFFFFFF),
              |tolerance| => float,
              |rect| => ((|x|, |y|), (|width|, |height|)) rect of ints,
                        or None */
/* Description: Returns count of color in bitmap.
                Functionally equivalent to:
                  {% len(find_every_color(color, tolerance, rect)) %} */
static PyObject *Bitmap_count_of_color(BitmapObject *self, PyObject *args);

/* Syntax: bmp.find_bitmap(needle, tolerance=0.0, rect=None) => tuple (x, y)
                                                                or None */
/* Arguments: |needle| => Bitmap object,
              |tolerance| => float,
              |rect| => ((|x|, |y|), (|width|, |height|)) rect of ints,
                        or None */
/* Description: Searches for |needle| in |bmp|. Returns tuple `(x, y)` of
                position if found, or None if not. */
static PyObject *Bitmap_find_bitmap(BitmapObject *self, PyObject *args);

/* Syntax: bmp.find_every_bitmap(needle, tolerance=0.0, rect=None) =>
                                         list of tuples [(x, y), ...] */
/* Arguments: |needle| => Bitmap object,
              |tolerance| => float,
              |rect| => ((|x|, |y|), (|width|, |height|)) rect of ints,
                        or None */
/* Description: Returns list of all `(x, y)` coordinates where |needle| occurs
                in |bmp|. */
static PyObject *Bitmap_find_every_bitmap(BitmapObject *self, PyObject *args);

/* Syntax: bmp.count_of_bitmap(needle, tolerance=0.0, rect=None) =>
                                         list of tuples [(x, y), ...] */
/* Arguments: |needle| => Bitmap object,
              |tolerance| => float,
              |rect| => ((|x|, |y|), (|width|, |height|)) rect of ints,
                        or None */
/* Description: Returns count of occurrences of |needle| in |haystack|.
                Functionally equivalent to:
                    {% len(bmp.find_every_bitmap(needle, tolerance)) %} */
static PyObject *Bitmap_count_of_bitmap(BitmapObject *self, PyObject *args);

/* Methods to make pixels in bitmap iterable.
 *
 * E.g., to get, say, the count of all white colors in an image, you could use:
 * `len([pixel for pixel in image if image.get_color(*pixel) == 0xFFUUUU])`
 * (which is, of course, just a slow version of `bmp.count_of_color(0xFFUUUU)`.)
 *
 * This should probably not be used in performance-critical code, but is quite
 * useful in debugging and general experimenting. */
static PyObject *Bitmap_iter(BitmapObject *self);
static PyObject *Bitmap_iternext(BitmapObject *self);

/* Returns a description of the Bitmap object. */
static PyObject *Bitmap_str(BitmapObject *self);

/* Getters/setters */

static PyObject *Bitmap_get_width(BitmapObject *self, PyObject *args);
static PyObject *Bitmap_get_height(BitmapObject *self, PyObject *args);

/* Deep copy method */
static PyObject *Bitmap_deepcopy(BitmapObject *self, PyObject *arg);

static PyGetSetDef Bitmap_getsetters[] = {
	{"width", (getter)Bitmap_get_width, NULL, NULL, NULL},
	{"height", (getter)Bitmap_get_height, NULL, NULL, NULL},
	{NULL} /* Sentinel */
};

static PyMethodDef Bitmap_methods[] = {
	{"__deepcopy__", (PyCFunction)Bitmap_deepcopy, METH_O, NULL},
	{"point_in_bounds", (PyCFunction)Bitmap_point_in_bounds, METH_VARARGS,
	 "Returns true if point is in bounds."},
	{"get_color", (PyCFunction)Bitmap_get_color, METH_VARARGS,
	 "bmp.get_color(x, y) -> hexadecimal integer\n"
	 "Returns hexadecimal value describing the RGB color at the given point."},
	{"open", Bitmap_open, METH_CLASS | METH_VARARGS,
	 "Bitmap.open(filepath) -> Bitmap object\n"
	 "Attempts to open image at the given absolute filepath."},
	{"from_string", Bitmap_from_string, METH_CLASS | METH_VARARGS,
	 "Bitmap.from_string(string) -> Bitmap object\n"
	 "Returns a bitmap object created from the given string."},
	{"get_portion", (PyCFunction)Bitmap_get_portion, METH_VARARGS,
	 "bmp.get_portion(x, y, width, height) -> Bitmap object\n"
	 "Returns new bitmap object created from portion of another."},
	{"copy_to_pboard", (PyCFunction)Bitmap_copy_to_pboard, METH_NOARGS,
	 "bmp.copy_to_pboard() -> None\n"
	 "Copies image to pasteboard."},
	{"save", (PyCFunction)Bitmap_save, METH_VARARGS,
	 "bmp.save(filepath, [format]) -> None\n"
	 "Saves bmp to absolute filepath in the given format.\n"
	 "If format is omitted, it is determined from the filename, if possible."},
	{"to_string", (PyCFunction)Bitmap_to_string, METH_NOARGS,
	 "bmp.to_string() -> string\n"
	 "Returns compressed, printable string representing bitmap."},
	{"find_color", (PyCFunction)Bitmap_find_color, METH_VARARGS,
	 "bmp.find_color((r, g, b), tolerance=0.0, rect=None) -> tuple (x, y) or "
	                                                         "None\n"
	 "Returns tuple (x, y) if color is found in given rect in bmp, or None if "
	 "not.\n"
	 "If rect is None, all of bmp is searched."},
	{"find_every_color", (PyCFunction)Bitmap_find_every_color, METH_VARARGS,
	 "bmp.find_every_color(color, tolerance=0.0, rect=None) -> "
	                 "list of tuples [(x, y), ...]\n"
	 "Returns list of all coordinates inside the given rect in bmp matching\n"
	 "the given color.\n"
	 "If rect is None, entire image is searched."},
	{"count_of_color", (PyCFunction)Bitmap_count_of_color, METH_VARARGS,
	 "bmp.count_of_color(color, tolerance=0.0, rect=None) -> integer\n"
	 "Returns count of color inside given rect in of bmp.\n"
	 "If rect is None, entire image is searched."},
	{"find_bitmap", (PyCFunction)Bitmap_find_bitmap, METH_VARARGS,
	 "bmp.find_bitmap(needle, tolerance=0.0, rect=None) -> tuple (x, y) "
	                                                       "or None\n"
	 "Returns tuple of coordinates if needle is found in given rect in bmp, "
	 "or None if not."},
	{"find_every_bitmap", (PyCFunction)Bitmap_find_every_bitmap, METH_VARARGS,
	 "bmp.find_every_bitmap(needle, tolerance=0.0, rect=None) -> "
	                   "list of tuples [(x, y), ...]\n"
	 "Returns list of all (x, y) coordinates where needle occurs in given "
	 "rect inside bmp."},
	{"count_of_bitmap", (PyCFunction)Bitmap_count_of_bitmap, METH_VARARGS,
	 "bmp.count_of_bitmap(needle, tolerance=0.0, rect=None) -> integer\n"
	 "Returns count of occurrences of needle in given rect inside bmp."},
	{NULL} /* Sentinel */
};

/* Ridiculous monstrosity */
PyTypeObject Bitmap_Type = {
   PyObject_HEAD_INIT(NULL)
   0,                             /* ob_size */
   "Bitmap",                      /* tp_name */
   sizeof(BitmapObject),          /* tp_basicsize */
   0,                             /* tp_itemsize */
   (destructor)Bitmap_dealloc,    /* tp_dealloc */
   0,                             /* tp_print */
   0,                             /* tp_getattr */
   0,                             /* tp_setattr */
   0,                             /* tp_compare */
   0,                             /* tp_repr */
   0,                             /* tp_as_number */
   0,                             /* tp_as_sequence */
   0,                             /* tp_as_mapping */
   0,                             /* tp_hash */
   0,                             /* tp_call */
   (reprfunc)Bitmap_str,          /* tp_str */
   0,                             /* tp_getattro */
   0,                             /* tp_setattro */
   0,                             /* tp_as_buffer */
   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
   Py_TPFLAGS_HAVE_ITER,          /* tp_flags*/
   "Raw, uncompressed bitmap object", /* tp_doc */
   0,                             /* tp_traverse */
   0,                             /* tp_clear */
   0,                             /* tp_richcompare */
   0,                             /* tp_weaklistoffset */
   (getiterfunc)Bitmap_iter,      /* tp_iter */
   (iternextfunc)Bitmap_iternext, /* tp_iternext */
   Bitmap_methods,                /* tp_methods */
   0,                             /* tp_members */
   Bitmap_getsetters,             /* tp_getset */
   0,                             /* tp_base */
   0,                             /* tp_dict */
   0,                             /* tp_descr_get */
   0,                             /* tp_descr_set */
   0,                             /* tp_dictoffset */
   0,                             /* tp_init */
   0,                             /* tp_alloc */
   PyType_GenericNew,             /* tp_new */
};

/* -- End of Bitmap class definition -- */

/* Helper function for |Bitmap_open()| and |Bitmap_save()|.
 *
 * Parses the arguments in f(filepath, [format]); that is, it sets |type| to
 * the corresponding MMImageType for |format| if |format| is given, or attempts
 * to infer the type from the filepath if not.
 *
 * Returns false and sets error if the image format could not be inferred. */
static bool parseImageIOArgs(PyObject *args, char **path, MMImageType *type);

/* -- Bitmap class method definitions -- */

static PyObject *Bitmap_open(PyObject *self, PyObject *args)
{
	char *path;
	MMImageType type;

	MMBitmapRef bitmap;
	MMIOError err;

	if (!parseImageIOArgs(args, &path, &type)) return NULL;
	if ((bitmap = newMMBitmapFromFile(path, type, &err)) == NULL) {
		PyErr_SetFormatString(PyExc_IOError, BUFSIZ,
		                      "Error opening image: %s",
		                      MMIOErrorString(type, err));
		return NULL;
	}

	return BitmapObject_FromMMBitmap(bitmap);
}

static PyObject *Bitmap_from_string(PyObject *self, PyObject *args)
{
	uint8_t *str;
	size_t len;

	MMBitmapRef bitmap;
	MMBMPStringError err;

	if (!PyArg_ParseTuple(args, "s#", &str, &len)) return NULL;
	if ((bitmap = createMMBitmapFromString(str, len, &err)) == NULL) {
		PyErr_SetFormatString(PyExc_ValueError, BUFSIZ,
		                      "Could not create bitmap from string: %s",
		                      MMBitmapStringErrorString(err));
		return NULL;
	}

	return BitmapObject_FromMMBitmap(bitmap);
}

/* -- Bitmap instance method definitions -- */

/* Returns false and sets error if |bitmap| is NULL. */
static bool Bitmap_Ready(BitmapObject *object)
{
	if (object->bitmap == NULL || object->bitmap->imageBuffer == NULL) {
		PyErr_SetString(PyExc_ValueError, "No image data set");
		return false;
	}
	return true;
}

/* Attempts to extract rect from tuple if |rectTuple| is not NULL, or uses
 * bounds of |bitmap| as a fallback. Returns false and sets error if rect could
 * not be extracted, or returns true if it could. */
static bool rectFromTupleOrBitmap(MMBitmapRef bitmap,
                                  PyObject *rectTuple,
                                  MMRect *rect);

static PyObject *Bitmap_str(BitmapObject *self)
{
	if (!Bitmap_Ready(self)) return NULL;
	return PyString_FromFormat("<Bitmap object at %p with resolution %lu%lu, "
	                           "%u bits per pixel, and %u bytes per pixel>",
	                           self,
	                           (unsigned long)self->bitmap->width,
	                           (unsigned long)self->bitmap->height,
	                           self->bitmap->bitsPerPixel,
	                           self->bitmap->bytesPerPixel);
}

static PyObject *Bitmap_deepcopy(BitmapObject *self, PyObject *arg)
{
	MMBitmapRef copy = self->bitmap == NULL ? NULL : copyMMBitmap(self->bitmap);
	return BitmapObject_FromMMBitmap(copy);
}

/* Creates new PyList from MMPointArray. */
static PyObject *PyList_FromPointArray(MMPointArrayRef pointArray);

/* -- Iterator methods -- */

static PyObject *Bitmap_iter(BitmapObject *self)
{
	if (!Bitmap_Ready(self)) return NULL;

	self->point = MMPointZero;
	Py_INCREF(self);
	return (PyObject *)self;
}

static PyObject *Bitmap_iternext(BitmapObject *self)
{
	if (!Bitmap_Ready(self)) return NULL;

	if (++(self->point).x < self->bitmap->width) {
		return Py_BuildValue("(kk)", self->point.x, self->point.y);
	} else if (++(self->point).y < self->bitmap->height){
		self->point.x = 0;
		return Py_BuildValue("(kk)", self->point.x, self->point.y);
	} else {
		/* Raise standard StopIteration exception with empty value. */
		PyErr_SetNone(PyExc_StopIteration);
		return NULL;
	}
}

/* -- End of iterator methods -- */

/* -- Getters/setters -- */

static PyObject *Bitmap_get_width(BitmapObject *self, PyObject *args)
{
	return Py_BuildValue("k", self->bitmap->width);
}

static PyObject *Bitmap_get_height(BitmapObject *self, PyObject *args)
{
	return Py_BuildValue("k", self->bitmap->height);
}

/* -- End of getters/setters -- */

static PyObject *Bitmap_get_portion(BitmapObject *self, PyObject *args)
{
	MMRect rect;
	MMBitmapRef portion = NULL;
	if (!PyArg_ParseTuple(args, "(kk)(kk)", &(rect.origin.x),
	                                        &(rect.origin.y),
	                                        &(rect.size.width),
	                                        &(rect.size.height))) {
		return NULL;
	}

	if (!MMBitmapRectInBounds(self->bitmap, rect)) {
		PyErr_SetString(PyExc_ValueError, "Portion out of bounds");
		return NULL;
	}

	portion = copyMMBitmapFromPortion(self->bitmap, rect);

	if (portion == NULL) {
		PyErr_SetString(PyExc_IOError, "Error grabbing bitmap portion");
		return NULL;
	}
	return BitmapObject_FromMMBitmap(portion);
}

static PyObject *Bitmap_point_in_bounds(BitmapObject *self, PyObject *args)
{
	MMPoint point;

	if (!PyArg_ParseTuple(args, "kk", &point.x, &point.y) ||
	    !Bitmap_Ready(self)) {
		return NULL;
	}

	if (MMBitmapPointInBounds(self->bitmap, point)) {
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject *Bitmap_copy_to_pboard(BitmapObject *self, PyObject *args)
{
	MMPasteError err;

	if (!Bitmap_Ready(self)) return NULL;
	if ((err = copyMMBitmapToPasteboard(self->bitmap)) != kMMPasteNoError) {
		PyErr_SetFormatString(PyExc_OSError, BUFSIZ,
		                      "Could not copy image to pasteboard: %s",
		                      MMPasteErrorString(err));
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *Bitmap_save(BitmapObject *self, PyObject *args)
{
	char *path;
	MMImageType type;

	if (!parseImageIOArgs(args, &path, &type) || !Bitmap_Ready(self)) {
		return NULL;
	}

	if (saveMMBitmapToFile(self->bitmap, path, type) != 0) {
		PyErr_SetString(PyExc_IOError, "Could not save image to file");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *Bitmap_to_string(BitmapObject *self, PyObject *args)
{
	char *buf = NULL;
	MMBMPStringError err;

	if (!Bitmap_Ready(self)) return NULL;

	if ((buf = (char *)createStringFromMMBitmap(self->bitmap, &err)) == NULL) {
		PyErr_SetFormatString(PyExc_IOError, BUFSIZ,
		                      "Could not create string from bitmap: %s",
		                      MMBitmapStringErrorString(err));
		return NULL;
	}
	return PyString_FromString(buf);
}

static PyObject *Bitmap_get_color(BitmapObject *self, PyObject *args)
{
	MMPoint point;

	if (!PyArg_ParseTuple(args, "kk", &point.x, &point.y) ||
	    !Bitmap_Ready(self)) return NULL;

	if (!MMBitmapPointInBounds(self->bitmap, point)) {
		PyErr_SetString(PyExc_ValueError, "Point out of bounds");
		return NULL;
	}

	return Py_BuildValue("I", MMRGBHexAtPoint(self->bitmap, point.x, point.y));
}

static PyObject *Bitmap_find_color(BitmapObject *self, PyObject *args)
{
	MMRGBHex color;
	float tolerance = 0.0f;
	PyObject *rectTuple = NULL;

	MMRect rect;
	MMPoint point;

	if (!PyArg_ParseTuple(args, "k|fO", &color, &tolerance, &rectTuple) ||
	    !Bitmap_Ready(self) ||
		!rectFromTupleOrBitmap(self->bitmap, rectTuple, &rect)) {
		return NULL;
	}

	if (findColorInRect(self->bitmap, color, &point, rect, tolerance) == 0) {
		return Py_BuildValue("(kk)", point.x, point.y);
	}

	Py_RETURN_NONE;
}

static PyObject *Bitmap_find_every_color(BitmapObject *self, PyObject *args)
{
	MMRGBHex color;
	float tolerance = 0.0f;
	PyObject *rectTuple = NULL;

	MMRect rect;
	MMPointArrayRef pointArray;
	PyObject *list;

	if (!PyArg_ParseTuple(args, "k|fO", &color, &tolerance, &rectTuple) ||
	    !Bitmap_Ready(self) ||
	    !rectFromTupleOrBitmap(self->bitmap, rectTuple, &rect)) {
		return NULL;
	}

	pointArray = findAllColorInRect(self->bitmap, color, rect, tolerance);
	if (pointArray == NULL) return NULL;

	list = PyList_FromPointArray(pointArray);
	destroyMMPointArray(pointArray);
	if (list == NULL) return NULL;

	return list;
}

static PyObject *Bitmap_count_of_color(BitmapObject *self, PyObject *args)
{
	MMRGBHex color;
	float tolerance = 0.0f;
	PyObject *rectTuple = NULL;

	MMRect rect;

	if (!PyArg_ParseTuple(args, "k|fO", &color, &tolerance, &rectTuple) ||
	    !Bitmap_Ready(self) ||
	    !rectFromTupleOrBitmap(self->bitmap, rectTuple, &rect)) {
		return NULL;
	}

	return Py_BuildValue("k", countOfColorsInRect(self->bitmap, color,
	                                              rect, tolerance));
}

static PyObject *Bitmap_find_bitmap(BitmapObject *self, PyObject *args)
{
	BitmapObject *needle;
	float tolerance = 0.0f;
	MMPoint point;
	PyObject *rectTuple = NULL;
	MMRect rect;

	if (!PyArg_ParseTuple(args, "O!|fO", &Bitmap_Type, &needle,
	                                     &tolerance, &rectTuple) ||
	    !Bitmap_Ready(self) || !Bitmap_Ready(needle) ||
	    !rectFromTupleOrBitmap(self->bitmap, rectTuple, &rect)) {
		return NULL;
	}

	if (findBitmapInRect(needle->bitmap, self->bitmap, &point,
	                     rect, tolerance) == 0) {
		return Py_BuildValue("(kk)", point.x, point.y);
	}

	Py_RETURN_NONE;
}

static PyObject *Bitmap_find_every_bitmap(BitmapObject *self, PyObject *args)
{
	BitmapObject *needle;
	float tolerance = 0.0f;
	PyObject *rectTuple = NULL;
	MMRect rect;

	MMPointArrayRef pointArray;
	PyObject *list;

	if (!PyArg_ParseTuple(args, "O!|fO", &Bitmap_Type, &needle,
	                                     &tolerance, &rectTuple) ||
	    !Bitmap_Ready(self) || !Bitmap_Ready(needle) ||
	    !rectFromTupleOrBitmap(self->bitmap, rectTuple, &rect)) {
		return NULL;
	}

	pointArray = findAllBitmapInRect(needle->bitmap, self->bitmap,
	                                 rect, tolerance);
	if (pointArray == NULL) return NULL;

	list = PyList_FromPointArray(pointArray);
	destroyMMPointArray(pointArray);
	if (list == NULL) return NULL;

	return list;
}

static PyObject *Bitmap_count_of_bitmap(BitmapObject *self, PyObject *args)
{
	BitmapObject *needle;
	float tolerance = 0.0f;
	PyObject *rectTuple = NULL;
	MMRect rect;

	if (!PyArg_ParseTuple(args, "O!|fO", &Bitmap_Type, &needle,
	                                     &tolerance, &rectTuple) ||
	    !Bitmap_Ready(self) || !Bitmap_Ready(needle) ||
	    !rectFromTupleOrBitmap(self->bitmap, rectTuple, &rect)) {
		return NULL;
	}

	return Py_BuildValue("k", countOfBitmapInRect(needle->bitmap, self->bitmap,
	                                              rect, tolerance));
}

static bool rectFromTupleOrBitmap(MMBitmapRef bitmap,
                                  PyObject *rectTuple,
                                  MMRect *rect)
{
	assert(rect != NULL);

	if (rectTuple == NULL || rectTuple == Py_None) {
		*rect = MMBitmapGetBounds(bitmap);
		return true;
	}

	if (!PyArg_ParseTuple(rectTuple, "(kk)(kk)", &(rect->origin.x),
	                                             &(rect->origin.y),
	                                             &(rect->size.width),
	                                             &(rect->size.height))) {
		PyErr_SetString(PyExc_TypeError, "Argument is not a rect");
		return false;
	}

	return true;
}

static PyObject *PyList_FromPointArray(MMPointArrayRef pointArray)
{
	PyObject *list;
	size_t i;
	assert(pointArray != NULL);

	if ((list = PyList_New(pointArray->count)) == NULL) return NULL;
	for (i = 0; i < pointArray->count; ++i) {
		MMPoint point = MMPointArrayGetItem(pointArray, i);
		PyObject *val = Py_BuildValue("(kk)", point.x, point.y);
		if (val == NULL) return NULL;

		PyList_SetItem(list, i, val); /* "Steals" the reference to val. */
	}

	return list;
}

static bool parseImageIOArgs(PyObject *args, char **path, MMImageType *type)
{
	int pathLen;
	char *format = NULL;

	assert(path != NULL);
	assert(type != NULL);

	if (!PyArg_ParseTuple(args, "s#|s", path, &pathLen, &format)) return false;

	/* Attempt to derive type from path */
	if (format == NULL) {
		const char *ext = getExtension(*path, pathLen);

		if (ext == NULL) {
			PyErr_SetString(PyExc_ValueError, "Could not find file extension");
			return false;
		}

		*type = imageTypeFromExtension(ext);
	} else {
		*type = imageTypeFromExtension(format);
	}

	if (*type == kInvalidImageType) {
		PyErr_SetString(PyExc_TypeError, "Unsupported image format");
		return false;
	}

	return true;
}

/* -- End of Bitmap instance methods -- */

PyObject *BitmapObject_FromMMBitmap(MMBitmapRef bitmap)
{
	BitmapObject *newBitmap = (BitmapObject *)_PyObject_New(&Bitmap_Type);
	if (newBitmap == NULL) return NULL;

	newBitmap->bitmap = bitmap;

	return (PyObject *)newBitmap;
}
