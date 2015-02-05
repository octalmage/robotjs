#pragma once
#ifndef KEYCODE_H
#define KEYCODE_H

#include "os.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#if defined(IS_MACOSX)

#include <Carbon/Carbon.h> /* Really only need <HIToolbox/Events.h> */
#include <ApplicationServices/ApplicationServices.h>

enum _MMKeyCode {
	K_BACKSPACE = kVK_Delete,
	K_DELETE = kVK_ForwardDelete,
	K_RETURN = kVK_Return,
	K_ESCAPE = kVK_Escape,
	K_UP = kVK_UpArrow,
	K_DOWN = kVK_DownArrow,
	K_RIGHT = kVK_RightArrow,
	K_LEFT = kVK_LeftArrow,
	K_HOME = kVK_Home,
	K_END = kVK_End,
	K_PAGEUP = kVK_PageUp,
	K_PAGEDOWN = kVK_PageDown,
	K_F1 = kVK_F1,
	K_F2 = kVK_F2,
	K_F3 = kVK_F3,
	K_F4 = kVK_F4,
	K_F5 = kVK_F5,
	K_F6 = kVK_F6,
	K_F7 = kVK_F7,
	K_F8 = kVK_F8,
	K_F9 = kVK_F9,
	K_F10 = kVK_F10,
	K_F11 = kVK_F11,
	K_F12 = kVK_F12,
	K_META = kVK_Command,
	K_ALT = kVK_Option,
	K_CONTROL = kVK_Control,
	K_SHIFT = kVK_Shift,
	K_CAPSLOCK = kVK_CapsLock
};

typedef CGKeyCode MMKeyCode;

#elif defined(USE_X11)

#include <X11/Xutil.h>

enum _MMKeyCode {
	K_BACKSPACE = XK_BackSpace,
	K_DELETE = XK_Delete,
	K_RETURN = XK_Return,
	K_ESCAPE = XK_Escape,
	K_UP = XK_Up,
	K_DOWN = XK_Down,
	K_RIGHT = XK_Right,
	K_LEFT = XK_Left,
	K_HOME = XK_Home,
	K_END = XK_End,
	K_PAGEUP = XK_Page_Up,
	K_PAGEDOWN = XK_Page_Down,
	K_F1 = XK_F1,
	K_F2 = XK_F2,
	K_F3 = XK_F3,
	K_F4 = XK_F4,
	K_F5 = XK_F5,
	K_F6 = XK_F6,
	K_F7 = XK_F7,
	K_F8 = XK_F8,
	K_F9 = XK_F9,
	K_F10 = XK_F10,
	K_F11 = XK_F11,
	K_F12 = XK_F12,
	K_META = XK_Super_L,
	K_ALT = XK_Alt_L,
	K_CONTROL = XK_Control_L,
	K_SHIFT = XK_Shift_L,
	K_CAPSLOCK = XK_Shift_Lock
};

typedef KeySym MMKeyCode;

#elif defined(IS_WINDOWS)

enum _MMKeyCode {
	K_BACKSPACE = VK_BACK,
	K_DELETE = VK_DELETE,
	K_RETURN = VK_RETURN,
	K_ESCAPE = VK_ESCAPE,
	K_UP = VK_UP,
	K_DOWN = VK_DOWN,
	K_RIGHT = VK_RIGHT,
	K_LEFT = VK_LEFT,
	K_HOME = VK_HOME,
	K_END = VK_END,
	K_PAGEUP = VK_PRIOR,
	K_PAGEDOWN = VK_NEXT,
	K_F1 = VK_F1,
	K_F2 = VK_F2,
	K_F3 = VK_F3,
	K_F4 = VK_F4,
	K_F5 = VK_F5,
	K_F6 = VK_F6,
	K_F7 = VK_F7,
	K_F8 = VK_F8,
	K_F9 = VK_F19,
	K_F10 = VK_F10,
	K_F11 = VK_F11,
	K_F12 = VK_F12,
	K_META = VK_LWIN,
	K_CONTROL = VK_CONTROL,
	K_SHIFT = VK_SHIFT,
	K_ALT = VK_MENU,
	K_CAPSLOCK = VK_CAPITAL
};

typedef int MMKeyCode;

#endif

/* Returns the keyCode corresponding to the current keyboard layout for the
 * given ASCII character. */
MMKeyCode keyCodeForChar(const char c);

#endif /* KEYCODE_H */

#ifdef __cplusplus
}
#endif
