autojs
========

Node.js GUI Automation.

##Installing##
```
npm install autojs
```

##Examples##
Get the mouse location and move it. 

```JavaScript
var autojs = require("autojs");

//Get the mouse position, retuns an object with x and y. 
var mouse=autojs.getMousePos();
console.log("Mouse is at x:" + mouse.x + " y:" + mouse.y);

//Move the mouse down by 100 pixels.
autojs.moveMouse(mouse.x,mouse.y+100);

//Left click!
autojs.mouseClick();
```
##Progress##

| Module        | Status        | Notes   |
| ------------- |-------------: | ------- |
| Mouse         | 80%           | Can't specify mouse button yet.       |
| Keyboard      | Not Started   | Send keypress, type string.        |
| Screen        | Not Started   | Screenshot, read pixel color, image search.        |
