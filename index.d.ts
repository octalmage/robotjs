export interface Bitmap {
  width: number
  height: number
  image: any
  byteWidth: number
  bitsPerPixel: number
  bytesPerPixel: number
  colorAt(x: number, y: number): string
}

export type KeyboardModifier = "alt" | "command" | "control" | "shift" | "none";

export interface Screen {
  capture(x?: number, y?: number, width?: number, height?: number): Bitmap
}

/**
 * change keyboard delay (default: 10)
 * @param delay keyboard delay in ms
 */
export function setKeyboardDelay(ms: number) : void
/**
 * 
 * @param key 
 * @param modifier 
 * @return taping time in ms
 */
export function keyTap(key: string, modifier?: KeyboardModifier | KeyboardModifier[]) : number
/**
 * 
 * @param key 
 * @param down 
 * @param modifier 
 * @return taping time in ms
 */
export function keyToggle(key: string, down: string, modifier?: KeyboardModifier | KeyboardModifier[]) : number
export function typeString(string: string) : void
export function typeStringDelayed(string: string, cpm: number) : number
/**
 * change mouse delay (default: 10)
 * @param delay mouse delay in ms
 */
export function setMouseDelay(delay: number) : void
/**
 * @param x 
 * @param y 
 * @param delay mouse delay in ms
 */
export function moveMouse(x: number, y: number) : number
/**
 * @param x 
 * @param y 
 * @param delay mouse delay in ms
 */
export function moveMouseSmooth(x: number, y: number) : number
export function mouseClick(button?: "left" | "right" | "middle", double?: boolean) : void
export function mouseToggle(down?: "down" | "up", button?: "left" | "right" | "middle") : void
export function dragMouse(x: number, y: number) : void
export function scrollMouse(x: number, y: number) : void
export function getMousePos(): { x: number, y: number }
export function getPixelColor(x: number, y: number): string
export function getScreenSize(): { width: number, height: number }
/**
 * X11 only return display name
 */
export function getXDisplayName(): String
/**
 * X11 only change display name
 */
export function setXDisplayName(String: name): number

export var screen: Screen
