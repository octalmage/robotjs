RobotJS
========

Node.js GUI Automation. Control the mouse, keyboard, and read the screen.

Based on [autopy](https://github.com/msanders/autopy). 

##Installing##
```
npm install robotjs
```

##Examples##
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
##Progress##

| Module        | Status        | Notes   |
| ------------- |-------------: | ------- |
| Mouse         | 80%           | Can't specify mouse button yet.       |
| Keyboard      | Not Started   | Send keypress, type string.        |
| Screen        | Not Started   | Screenshot, read pixel color, image search.        |
