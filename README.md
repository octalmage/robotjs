RobotJS
========

> Node.js Desktop Automation. Control the mouse, keyboard, and read the screen.

RobotJS supports Mac, [Windows](https://github.com/octalmage/robotjs/issues/2), and [Linux](https://github.com/octalmage/robotjs/issues/17).

This is a work in progress so the exported functions could change at any time before the first stable release (1.0.0). [Ideas?](https://github.com/octalmage/robotjs/issues/4)

[Check out some of the cool things people are making with  RobotJS](https://github.com/octalmage/robotjs/wiki/Projects-using-RobotJS)! Have your own rad RobotJS project? Feel free to add it!

[![npm version](https://img.shields.io/npm/v/robotjs.svg)](https://www.npmjs.com/package/robotjs) [![Build Status](https://api.travis-ci.org/octalmage/robotjs.svg?branch=master)](https://travis-ci.org/octalmage/robotjs) [![Ready](https://badge.waffle.io/octalmage/robotjs.svg?label=ready&title=Ready)](http://waffle.io/octalmage/robotjs) [![Join the chat at https://gitter.im/octalmage/robotjs](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/octalmage/robotjs?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Installing

Please make sure you have the [required dependencies](https://github.com/TooTallNate/node-gyp/#installation) before installing:

* Windows
  * Visual Studio 2013 (Express works fine).
  * Python (v2.7.3 recommended, v3.x.x is not supported).
* Mac
  * Xcode Command Line Tools.
* Linux
  * Python (v2.7 recommended, v3.x.x is not supported).
  * make.
  * A C/C++ compiler like GCC.
  * libxtst-dev and libpng++-dev (`sudo apt-get install libxtst-dev libpng++-dev`).

Then install RobotJS using npm:

```
npm install robotjs
```
I [plan on](https://github.com/octalmage/robotjs/issues/64) using node-pre-gyp to make this process easier.

## Examples
Get the mouse position, move it, then click.

```JavaScript
var robot = require("robotjs");

//Get the mouse position, returns an object with x and y. 
var mouse = robot.getMousePos();
console.log("Mouse is at x:" + mouse.x + " y:" + mouse.y);

//Move the mouse down by 100 pixels.
robot.moveMouse(mouse.x, mouse.y + 100);

//Left click!
robot.mouseClick();
```

Type "Hello World" then press enter.

```JavaScript
var robot = require("robotjs");

//Type "Hello World".
robot.typeString("Hello World");

//Press enter. 
robot.keyTap("enter");
```
Get pixel color under the mouse. 

```JavaScript
var robot = require("robotjs");

//Get mouse position. 
var mouse = robot.getMousePos();

//Get pixel color in hex format. 
var hex = robot.getPixelColor(mouse.x, mouse.y);
console.log("#" + hex + " at x:" + mouse.x + " y:" + mouse.y);
```

Read the [Wiki](https://github.com/octalmage/robotjs/wiki) for more information!

## Building

node-gyp is required to build RobotJS.

Install node-gyp using npm:

```
npm install -g node-gyp
```

Then configure and build: 

```
node-gyp configure
node-gyp build
```

## Plans

* Control the mouse by changing the mouse position, left/right clicking, and dragging. 
* Control the keyboard by pressing keys, holding keys down, and typing words.
* Read pixel color from the screen and capture the screen. 
* Find image on screen, read pixels from image.
* Possibly include window management? 

## Progress

| Module        | Status        | Notes   |
| ------------- |-------------: | ------- |
| Mouse         | 100%           | All planned features implemented.       |
| Keyboard      | 100%           | All planned features implemented.       |
| Screen        | 10%            | Screenshot, image search.   |

## Story

I'm a huge fan of [AutoHotkey](http://www.autohotkey.com/), and I've used it for a very long time. AutoHotkey is great for automation and it can do a bunch of things that are very difficult in other languages. For example, it's [imagesearch](https://www.autohotkey.com/docs/commands/ImageSearch.htm) and [pixel](https://www.autohotkey.com/docs/commands/PixelGetColor.htm) related functions are hard to reproduce on Mac, especially in scripting languages. These functions are great for automating apps that can't be automated like [Netflix](http://blueshirtdesign.com/apps/autoflix/). This has never been a big deal since I've always used Windows at work, but for the past few years I've been using Mac exclusively. 

I like AutoHotkey, but I like Node.js more. By developing RobotJS I get an AutoHotkey replacement on Mac (finally!), and I get to use my favorite language. 

**TLDR:** There's nothing like AutoHotkey on Mac, so I'm making it. 

## License

MIT

Based on [autopy](https://github.com/msanders/autopy). 
Maintained by [Jason Stallings](http://jason.stallin.gs).
