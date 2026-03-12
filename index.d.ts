export interface Bitmap
{
  width: number;
  height: number;
  image: any;
  byteWidth: number;
  bitsPerPixel: number;
  bytesPerPixel: number;
  colorAt(x: number, y: number): string;
}

export interface Screen
{
  capture(x?: number, y?: number, width?: number, height?: number): Bitmap;
}

type keys = "backspace" | "delete" | "enter" | "tab" | "escape" | "up" |
  "down" | "left" | "right" | "home" | "end" | "pageup" | "pagedown" | "f1" |
  "f2" | "f3" | "f4" | "f5" | "f6" | "f7" | "f8" | "f9" | "f10" | "f11" | "f12" |
  "command" | "alt" | "control" | "shift" | "right_shift" | "space" | "printscreen" |
  "insert" | "audio_mute" | "audio_vol_up" | "audio_vol_down" | "audio_play" | "audio_stop" |
  "audio_pause" | "audio_prev" | "audio_next" | "audio_rewind" | "audio_forward" | "audio_repeat" |
  "audio_random" | "numpad_0" | "numpad_1" | "numpad_2" | "numpad_3" | "numpad_4" | "numpad_5" |
  "numpad_6" | "numpad_7" | "numpad_8" | "numpad_9" | "lights_mon_up" | "lights_mon_down" |
  "lights_kbd_toggle" | "lights_kbd_up" | "lights_kbd_down";

type mouseButton = "left" | "right" | "middle";

export function setKeyboardDelay(ms: number): void;
export function keyTap(key: keys, modifier?: string | string[]): void;
export function keyToggle(key: keys, down: string, modifier?: string | string[]): void;
export function unicodeTap(value: number): void;
export function typeString(string: string): void;
export function typeStringDelayed(string: string, cpm: number): void;
export function setMouseDelay(delay: number): void;
export function updateScreenMetrics(): void;
export function moveMouse(x: number, y: number): void;
export function moveMouseSmooth(x: number, y: number, speed?: number): void;
export function mouseClick(button?: mouseButton, double?: boolean): void;
export function mouseToggle(down?: "down" | "up", button?: mouseButton): void;
export function dragMouse(x: number, y: number): void;
export function scrollMouse(x: number, y: number): void;
export function getMousePos(): { x: number, y: number; };
export function getPixelColor(x: number, y: number): string;
export function getScreenSize(): { width: number, height: number; };

export var screen: Screen;
