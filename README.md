autonode
========

Node.js GUI Automation.

##Building##
```
node-gyp configure build
```

##Examples##
Get the mouse location and move it. 

```JavaScript
var autonode = require("autonode.node");

//Get the mouse position, retuns an object with x and y. 
var mouse=autonode.getMousePos();
console.log("Mouse is at x:" + mouse.x + " y:" + mouse.y);

//Move the mouse down by 100 pixels.
autonode.moveMouse(mouse.x,mouse.y+100);
```
##Progress##

| Module        | Status        | Notes   |
| ------------- |-------------: | ------- |
| Mouse         | 50%           | Missing mouse click functions.        |
| Keyboard      | Not Started   | Send keypress, type string.        |
| Screen        | Not Started   | Screenshot, read pixel color, image search.        |
