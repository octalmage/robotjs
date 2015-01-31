var robotjs = require('./build/Release/robotjs.node');

//Objects.
var mouse = {}, keyboard = {};

//Mouse.
mouse.click = robotjs.mouseClick;
mouse.move = robotjs.moveMouse;
mouse.pos = robotjs.getMousePos;

//Keyboard.
keyboard.tap = robotjs.keyTap;
keyboard.type = robotjs.typeString;

//Exports.
module.exports.mouse = mouse;
module.exports.keyboard = keyboard;