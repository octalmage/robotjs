#include "autopy-mouse-module.h"
#include "mouse.h"
#include "py-convenience.h"
#include "deadbeef_rand.h"
#include "screen.h"

/* Syntax: move(x, y) */
/* Arguments: |x| => int,
              |y| => int */
/* Description: Moves the mouse to the given `(x, y)` coordinate. */
/* Raises: |ValueError| if the point is out of index. */
static PyObject *mouse_move(PyObject *self, PyObject *args);

/* Syntax: smooth_move(x, y) */
/* Arguments: |x| => int,
              |y| => int */
/* Description: Smoothly moves the mouse to the given `(x, y)` coordinate in a
                straight line. */
/* Raises: |ValueError| if the point is out of index. */
static PyObject *mouse_smooth_move(PyObject *self, PyObject *args);

/* Syntax: get_pos() => tuple (x, y) */
/* Description: Returns a tuple `(x, y)` of the current mouse position. */
static PyObject *mouse_get_pos(PyObject *self, PyObject *args);

/* Syntax: toggle(down, button=LEFT_BUTTON) */
/* Arguments: |down| => Boolean
              |button| => int */
/* Description: Holds down or releases the given mouse button in the current
                position. |button| can be |LEFT_BUTTON|, |RIGHT_BUTTON|, or
                |CENTER_BUTTON|. If no button is given, the left button is
                defaulted to. */
/* Raises: |ValueError| if the mouse button is invalid. */
static PyObject *mouse_toggle(PyObject *self, PyObject *args);

/* Syntax: click(button=LEFT_BUTTON) */
/* Arguments: |button| => int */
/* Description: Convenience wrapper around `toggle()` that holds down and then
                releases the given mouse button. */
static PyObject *mouse_click(PyObject *self, PyObject *args);

static PyMethodDef MouseMethods[] = {
	{"move", mouse_move, METH_VARARGS,
	 "move(x, y) -> None\n"
	 "Moves cursor to given (x, y) coordinate."},
	{"smooth_move", mouse_smooth_move, METH_VARARGS,
	 "smooth_move(x, y) -> None\n"
	 "Smoothly moves cursor to given (x, y) coordinate in a straight line."},
	{"get_pos", mouse_get_pos, METH_NOARGS,
	 "get_pos() -> tuple\n"
	 "Returns a tuple (x, y) of the current mouse position."},
	{"toggle", mouse_toggle, METH_VARARGS,
	 "toggle(down, button=LEFT_BUTTON) -> None\n"
	 "Holds down or releases the mouse with the given button."},
	{"click", mouse_click, METH_VARARGS,
	 "click(button=LEFT_BUTTON) -> None\n"
	 "Clicks the mouse with the given button."},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

PyMODINIT_FUNC initmouse(void)
{
	PyObject *mod = Py_InitModule3("mouse", MouseMethods,
	                               "autopy module for working with the mouse");

	if (mod == NULL) return; /* Error */

	/* Add mouse button constants for click_mouse(). */
	if (PyModule_AddIntMacro(mod, LEFT_BUTTON) < 0 ||
		PyModule_AddIntMacro(mod, RIGHT_BUTTON) < 0 ||
		PyModule_AddIntMacro(mod, CENTER_BUTTON) < 0) {
		PyErr_SetString(PyExc_ValueError, "Error adding constants to mouse module");
		return;
	}

	deadbeef_srand_time();
}

static PyObject *mouse_move(PyObject *self, PyObject *args)
{
	MMPoint point;
	if (!PyArg_ParseTuple(args, "kk", &point.x, &point.y)) return NULL;

	if (!pointVisibleOnMainDisplay(point)) {
		PyErr_SetString(PyExc_ValueError, "Point out of bounds");
		return NULL;
	}

	moveMouse(point);

	Py_RETURN_NONE;
}

static PyObject *mouse_smooth_move(PyObject *self, PyObject *args)
{
	MMPoint point;
	if (!PyArg_ParseTuple(args, "kk", &point.x, &point.y)) return NULL;

	if (!pointVisibleOnMainDisplay(point) || !smoothlyMoveMouse(point)) {
		PyErr_SetString(PyExc_ValueError, "Point out of bounds");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *mouse_get_pos(PyObject *self, PyObject *args)
{
	MMPoint pos = getMousePos();
	return Py_BuildValue("kk", pos.x, pos.y);
}

static PyObject *mouse_toggle(PyObject *self, PyObject *args)
{
	PyObject *downBool;
	MMMouseButton button = LEFT_BUTTON;

	if (!PyArg_ParseTuple(args, "O!|I", &PyBool_Type, &downBool, &button)) {
		return NULL;
	}

	if (!MMMouseButtonIsValid(button)) {
		PyErr_SetString(PyExc_ValueError, "Invalid mouse button");
		return NULL;
	}

	toggleMouse(downBool == Py_True, button);

	Py_RETURN_NONE;
}

static PyObject *mouse_click(PyObject *self, PyObject *args)
{
	MMMouseButton button = LEFT_BUTTON;

	if (!PyArg_ParseTuple(args, "|I", &button)) return NULL;

	if (!MMMouseButtonIsValid(button)) {
		PyErr_SetString(PyExc_ValueError, "Invalid mouse button");
		return NULL;
	}

	clickMouse(button);

	Py_RETURN_NONE;
}
