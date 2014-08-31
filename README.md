autonode
========

Node.js GUI Automation.

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
