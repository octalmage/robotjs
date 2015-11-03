![](https://cldup.com/1ATDf2JMtv.png)
========

[![Build Status](https://api.travis-ci.org/octalmage/robotjs.svg?branch=master)](https://travis-ci.org/octalmage/robotjs) [![Build status](https://ci.appveyor.com/api/projects/status/qh2eqb37j7ap6x36?svg=true)](https://ci.appveyor.com/project/octalmage/robotjs) [![npm version](https://img.shields.io/npm/v/robotjs.svg)](https://www.npmjs.com/package/robotjs) [![Join the chat at https://gitter.im/octalmage/robotjs](https://img.shields.io/badge/gitter-join%20chat-blue.svg)](https://gitter.im/octalmage/robotjs?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Wat: Documented](https://img.shields.io/badge/wat-documented-blue.svg)](https://github.com/dthree/wat)

> Node.js Desktop Automation. Control the mouse, keyboard, and read the screen.

RobotJS supports Mac, [Windows](https://github.com/octalmage/robotjs/issues/2), and [Linux](https://github.com/octalmage/robotjs/issues/17).

This is a work in progress so the exported functions could change at any time before the first stable release (1.0.0). [Ideas?](https://github.com/octalmage/robotjs/issues/4)

[Check out some of the cool things people are making with  RobotJS](https://github.com/octalmage/robotjs/wiki/Projects-using-RobotJS)! Have your own rad RobotJS project? Feel free to add it!

## Contents

- [Installation](#installation)
- [Examples](#examples)
- [API](https://github.com/octalmage/robotjs/wiki/Syntax)
- [Building](#building)
- [Plans](#plans)
- [Progress](#progress)
- [FAQ](#faq)
- [Blog](#blog)
- [License](#license)

## Installation

Please ensure you have the [required dependencies](https://github.com/TooTallNate/node-gyp/#installation) before installing:

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

##### Mouse

![](https://cloudup.com/cw5JY2cusx3+)

```JavaScript
//Move the mouse across the screen as a sine wave.
var robot = require("robotjs");

//Speed up the mouse.
robot.setMouseDelay(2);

var twoPI = Math.PI * 2.0;
var screenSize = robot.getScreenSize();
var height = (screenSize.height / 2) - 10;
var width = screenSize.width;

for (var x = 0; x < width; x++)
{
	y = height * Math.sin((twoPI * x) / width) + height;
	robot.moveMouse(x, y);
}
```

##### Keyboard

```JavaScript
//Type "Hello World" then press enter.
var robot = require("robotjs");

//Type "Hello World".
robot.typeString("Hello World");

//Press enter. 
robot.keyTap("enter");
```

##### Screen

```JavaScript
//Get pixel color under the mouse. 
var robot = require("robotjs");

//Get mouse position. 
var mouse = robot.getMousePos();

//Get pixel color in hex format. 
var hex = robot.getPixelColor(mouse.x, mouse.y);
console.log("#" + hex + " at x:" + mouse.x + " y:" + mouse.y);
```
Read the [Wiki](https://github.com/octalmage/robotjs/wiki) for more information!

## [API](https://github.com/octalmage/robotjs/wiki/Syntax)

The [RobotJS API](https://github.com/octalmage/robotjs/wiki/Syntax) is contained in the [Wiki](https://github.com/octalmage/robotjs/wiki).

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

## FAQ

#### Does RobotJS support global hotkeys? 

Not currently, and I don't know if it ever will. I personally use [Electron](http://electron.atom.io/)/[NW.js](http://nwjs.io/) for global hotkeys, and this works well. Later on I might add hotkey support or create a separate module. See [#55](https://github.com/octalmage/robotjs/issues/55) for details. 

#### Can I take a screenshot with RobotJS? 

Soon! This is a bit more complicated than the rest of the features, so I saved it for last. Luckily the code is already there, I just need to write the bindings, and I've already started. Subscribe to [#13](https://github.com/octalmage/robotjs/issues/13) for updates. 

#### Why is &#60;insert key&#62; missing from the keyboard functions? 

We've been implementing keys as we need them. Feel free to create an issue or submit a pull request!

For any other questions please [submit an issue](https://github.com/octalmage/robotjs/issues/new).

## <a href="http://robotjavascript.tumblr.com/">Blog</a>

For the latest RobotJS news, check out the [blog](http://robotjavascript.tumblr.com)!

<a href="http://robotjavascript.tumblr.com" target="_blank"><img src="http://feeds.feedburner.com/RobotjsOfficialBlog.1.gif"></a>

## Story

I'm a huge fan of [AutoHotkey](http://www.autohotkey.com/), and I've used it for a very long time. AutoHotkey is great for automation and it can do a bunch of things that are very difficult in other languages. For example, it's [imagesearch](https://www.autohotkey.com/docs/commands/ImageSearch.htm) and [pixel](https://www.autohotkey.com/docs/commands/PixelGetColor.htm) related functions are hard to reproduce on Mac, especially in scripting languages. These functions are great for automating apps that can't be automated like [Netflix](http://blueshirtdesign.com/apps/autoflix/). This has never been a big deal since I've always used Windows at work, but for the past few years I've been using Mac exclusively. 

I like AutoHotkey, but I like Node.js more. By developing RobotJS I get an AutoHotkey replacement on Mac (finally!), and I get to use my favorite language. 

**TLDR:** There's nothing like AutoHotkey on Mac, so I'm making it. 

## License

MIT

Based on [autopy](https://github.com/msanders/autopy). 
Maintained by [Jason Stallings](http://jason.stallin.gs).
