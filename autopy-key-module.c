#include "autopy-key-module.h"
#include "keypress.h"
#include "py-convenience.h"
#include "deadbeef_rand.h"
#include <assert.h>

#if defined(_MSC_VER)
	#include "ms_stdbool.h"
#else
	#include <stdbool.h>
#endif

/* Syntax: toggle(key, down_or_up, [modifiers]) */
/* Arguments: |key| => character or integer describing the key or keycode,
              |down_or_up| => Boolean describing whether to hold down or
                              release the key,
              |modifiers| => integer (bitwise-OR of |MOD_*| constants) */
/* Description: Holds down the given key or keycode if |down_or_up| is True,
				or releases it if not. Integer keycodes and modifiers should
				be taken from module constants (e.g., |key.K_DELETE| or
				|key.MOD_META|). If the given key is a character, it is
				automatically converted to a keycode corresponding to the
				current keyboard layout.

				To hold down more than one modifier at a time, bitwise-OR them,
				e.g.:
                    {% toggle('a', True, key.MOD_SHIFT | key.MOD_ALT) %} */
static PyObject *key_toggle(PyObject *self, PyObject *args);

/* Syntax: tap(key, [modifiers]) */
/* Arguments: |key| => character or integer,
              |modifiers| => integer (bitise-OR of |MOD_*| constants) */
/* Description: Convenience wrapper around `toggle()` that holds down and then
                releases the given key and modifiers. */
static PyObject *key_tap(PyObject *self, PyObject *args);

/* Syntax: type_string(string, wpm=0) */
/* Arguments: |string| => ASCII string,
              |wpm| => double (Words per minute to type string,
                               or unlimited if 0) */
/* Description: Attempts to simulate typing a string at the given WPM, or as
                fast as possible if the WPM is 0. */
static PyObject *key_type_string(PyObject *self, PyObject *args);

static PyMethodDef KeyMethods[] = {
	{"toggle", key_toggle, METH_VARARGS,
	 "toggle(key, down_or_up, [modifiers]) -> None\n"
	 "Holds down given key if down_or_up is True, releases if not."},
	{"tap", key_tap, METH_VARARGS,
	 "tap(key, [modifiers]) -> None\n"
	 "Holds down and then releases the given key."},
	{"type_string", key_type_string, METH_VARARGS,
	 "type_string(string, wpm=0) -> None\n"
	 "Attempts to type a string at the given WPM, or as fast as possible if\n"
	 "wpm is 0."},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

PyMODINIT_FUNC initkey(void)
{
	PyObject *mod = Py_InitModule3("key", KeyMethods,
	                               "autopy module for working with the "
	                               "keyboard");
	if (mod == NULL) return; /* Error */

	/* Needed for type_string(). */
	deadbeef_srand_time();

	/* Add keycode constants for toggle() and tap(). */
	if (PyModule_AddIntMacro(mod, MOD_NONE) < 0 ||
		PyModule_AddIntMacro(mod, MOD_META) < 0 ||
		PyModule_AddIntMacro(mod, MOD_ALT) < 0 ||
		PyModule_AddIntMacro(mod, MOD_CONTROL) < 0 ||
		PyModule_AddIntMacro(mod, MOD_SHIFT) < 0 ||
		PyModule_AddIntMacro(mod, K_BACKSPACE) < 0 ||
		PyModule_AddIntMacro(mod, K_DELETE) < 0 ||
		PyModule_AddIntMacro(mod, K_RETURN) < 0 ||
		PyModule_AddIntMacro(mod, K_ESCAPE) < 0 ||
		PyModule_AddIntMacro(mod, K_UP) < 0 ||
		PyModule_AddIntMacro(mod, K_DOWN) < 0 ||
		PyModule_AddIntMacro(mod, K_RIGHT) < 0 ||
		PyModule_AddIntMacro(mod, K_LEFT) < 0 ||
		PyModule_AddIntMacro(mod, K_HOME) < 0 ||
		PyModule_AddIntMacro(mod, K_END) < 0 ||
		PyModule_AddIntMacro(mod, K_PAGEUP) < 0 ||
		PyModule_AddIntMacro(mod, K_PAGEDOWN) < 0 ||
		PyModule_AddIntMacro(mod, K_F1) < 0 ||
		PyModule_AddIntMacro(mod, K_F2) < 0 ||
		PyModule_AddIntMacro(mod, K_F3) < 0 ||
		PyModule_AddIntMacro(mod, K_F4) < 0 ||
		PyModule_AddIntMacro(mod, K_F5) < 0 ||
		PyModule_AddIntMacro(mod, K_F6) < 0 ||
		PyModule_AddIntMacro(mod, K_F7) < 0 ||
		PyModule_AddIntMacro(mod, K_F8) < 0 ||
		PyModule_AddIntMacro(mod, K_F9) < 0 ||
		PyModule_AddIntMacro(mod, K_F10) < 0 ||
		PyModule_AddIntMacro(mod, K_F11) < 0 ||
		PyModule_AddIntMacro(mod, K_F12) < 0 ||
		PyModule_AddIntMacro(mod, K_META) < 0 ||
		PyModule_AddIntMacro(mod, K_ALT) < 0 ||
		PyModule_AddIntMacro(mod, K_CONTROL) < 0 ||
		PyModule_AddIntMacro(mod, K_SHIFT) < 0 ||
		PyModule_AddIntMacro(mod, K_CAPSLOCK) < 0) {
		PyErr_SetString(PyExc_ValueError, "Error adding keycode constants");
		return;
	}
}

/*  Attempts to extract MMKeyCode from PyInt. Returns false and sets error if
 *  MMKeyCode could not be converted, or returns true if it could. */
static bool MMKeyCodeFromPyInt(PyObject *num, MMKeyCode *val);

/*  Attempts to extract char from PyString. Returns false and sets error if
 *  MMKeyCode could not be converted, or returns true if it could. */
static bool charFromPyString(PyObject *str, char *val);

static PyObject *key_toggle(PyObject *self, PyObject *args)
{
	PyObject *key;
	PyObject *downBool;
	MMKeyFlags flags = MOD_NONE;

	if (!PyArg_ParseTuple(args, "OO!|K", &key, &PyBool_Type, &downBool, &flags)) {
		return NULL;
	}

	if (PyNumber_Check(key)) { /* Check for key code */
		MMKeyCode code;
		if (!MMKeyCodeFromPyInt(key, &code)) return NULL;
		toggleKeyCode(code, downBool == Py_True, flags);
	} else if (PyString_Check(key)) { /* Check for single-character string */
		char c;
		if (!charFromPyString(key, &c)) return NULL;
		toggleKey(c, downBool == Py_True, flags);
	} else {
		return Py_SetArgConvertErr("integer<H> or char", 1, key);
	}

	Py_RETURN_NONE;
}

static PyObject *key_tap(PyObject *self, PyObject *args)
{
	PyObject *key;
	MMKeyFlags flags = MOD_NONE;

	if (!PyArg_ParseTuple(args, "O|I", &key, &flags)) return NULL;

	if (PyNumber_Check(key)) { /* Check for key code */
		MMKeyCode code;
		if (!MMKeyCodeFromPyInt(key, &code)) return NULL;
		tapKeyCode(code, flags);
	} else if (PyString_Check(key)) { /* Check for single-character string */
		char c;
		if (!charFromPyString(key, &c)) return NULL;
		tapKey(c, flags);
	} else {
		return Py_SetArgConvertErr("integer<H> or char", 1, key);
	}

	Py_RETURN_NONE;
}

static PyObject *key_type_string(PyObject *self, PyObject *args)
{
	char *str = NULL;
	double wpm = 0.0;
	if (!PyArg_ParseTuple(args, "s|d", &str, &wpm)) return NULL;

	if (wpm == 0.0) {
		typeString(str);
	} else {
		typeStringDelayed(str, WPM_TO_CPM(wpm));
	}

	Py_RETURN_NONE;
}

/* -- Helper functions -- */

static bool MMKeyCodeFromPyInt(PyObject *num, MMKeyCode *val)
{
	long ival;
	assert(val != NULL);

	if (!PyInt_Check(num) || PyFloat_Check(num)) {
		Py_SetConvertErr("integer<H>", num);
		return false;
	}

	ival = PyInt_AsUnsignedLongMask(num);
	if (ival == -1 && PyErr_Occurred()) {
		Py_SetConvertErr("integer<H>", num);
		return false;
	}

	*val = (MMKeyCode)ival;
	return true;
}

static bool charFromPyString(PyObject *str, char *val)
{
	assert(val != NULL);
	if (PyString_Size(str) != 1) {
		Py_SetConvertErr("char", str);
		return false;
	} else {
		*val = PyString_AS_STRING(str)[0];
		return true;
	}
}
