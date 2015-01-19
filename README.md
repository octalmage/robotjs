RobotJS
========

Node.js GUI Automation. Control the mouse, keyboard, and read the screen.

This is a work in progress so the exported functions could change at any time before the first stable release (1.0.0). 

Only tested on Mac OS X using Node.js v0.10.31. I'm working on rewriting RobotJS using [nan](https://github.com/rvagg/nan) to support other versions of Node.js. node-gyp is only configured for Mac, but this project will support Windows and Linux when I have time to test.

Based on [autopy](https://github.com/msanders/autopy). 

##Installing

```
npm install robotjs
```

##Examples
Get the mouse location and move it. 

```JavaScript
var robot = require("robotjs");

//Get the mouse position, retuns an object with x and y. 
var mouse=robot.getMousePos();
console.log("Mouse is at x:" + mouse.x + " y:" + mouse.y);

//Move the mouse down by 100 pixels.
robot.moveMouse(mouse.x,mouse.y+100);

//Left click!
robot.mouseClick();
```

##Building

RobotJS uses [node-gyp](https://github.com/TooTallNate/node-gyp) for building. 

Install node-gyp using npm:

```
npm install -g node-gyp
```

Then confgure and build: 

```
node-gyp configure
node-gyp build
```

##Progress

| Module        | Status        | Notes   |
| ------------- |-------------: | ------- |
| Mouse         | 80%           | Can't specify mouse button.       |
| Keyboard      | 50%           | No toggle support.       |
| Screen        | 5%            | Screenshot, read pixel color, image search.        |
| Window        | 1%            | Manipulate external windows.     |
