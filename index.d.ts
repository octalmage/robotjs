/// <reference types="node" />

export interface Point {
  x: number
  y: number
}

export interface ScreenPoint {
  x: number
  y: number
}

export interface BitmapSearchOptions {
  x?: number
  y?: number
  width?: number
  height?: number
  tolerance?: number
}

export type ImageSearchOptions = BitmapSearchOptions

export interface Image {
  width: number
  height: number
  image: Buffer
  byteWidth: number
  bitsPerPixel: number
  bytesPerPixel: number
  screenX?: number
  screenY?: number
  scaleX?: number
  scaleY?: number
  colorAt(x: number, y: number): string
  findColor(color: string, options?: ImageSearchOptions): Point | null
  findColors(color: string, options?: ImageSearchOptions): Point[]
  countColor(color: string, options?: ImageSearchOptions): number
  findImage(needle: Image, options?: ImageSearchOptions): Point | null
  findImages(needle: Image, options?: ImageSearchOptions): Point[]
  countImage(needle: Image, options?: ImageSearchOptions): number
  save(path: string): boolean
  toScreenPoint(point: Point, target?: { width: number, height: number }): ScreenPoint
  click(point: Point, target?: { width: number, height: number }, button?: string, double?: boolean): ScreenPoint
  clickImage(target: Image, options?: ImageSearchOptions, button?: string, double?: boolean): Point | null
}

export declare class Image {
  constructor(
    width: number,
    height: number,
    byteWidth: number,
    bitsPerPixel: number,
    bytesPerPixel: number,
    image: Buffer
  )
}

export type Bitmap = Image
export declare const Bitmap: typeof Image

export interface Screen {
  capture(x?: number, y?: number, width?: number, height?: number): Image
}

export interface ImageModule {
  load(path: string): Image
  save(bitmap: Image, path: string): boolean
  supportsPNG: boolean
}

export function setKeyboardDelay(ms: number) : void
export function keyTap(key: string, modifier?: string | string[]) : void
export function keyToggle(key: string, down: string, modifier?: string | string[]) : void
export function unicodeTap(value: number) : void
export function typeString(string: string) : void
export function typeStringDelayed(string: string, cpm: number) : void
export function setMouseDelay(delay: number) : void
export function updateScreenMetrics() : void
export function moveMouse(x: number, y: number) : void
export function moveMouseSmooth(x: number, y: number,speed?:number) : void
export function mouseClick(button?: string, double?: boolean) : void
export function mouseToggle(down?: string, button?: string) : void
export function dragMouse(x: number, y: number) : void
export function scrollMouse(x: number, y: number) : void
export function getMousePos(): { x: number, y: number }
export function getPixelColor(x: number, y: number): string
export function getScreenSize(): { width: number, height: number }

export var screen: Screen
export var image: ImageModule
