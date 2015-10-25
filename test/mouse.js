var test = require('tape');
var robot = require('..');
var lastKnownPos, currentPos;
//var detectMouse = require('./detectMouse.js');

//Increase delay to help test reliability.
robot.setMouseDelay(100);

test('Get the initial mouse position.', function(t) 
{
	t.plan(3);
	t.ok(lastKnownPos = robot.getMousePos(), 'successfully retrieved mouse position.');
	t.ok(lastKnownPos.x !== undefined, 'mousepos.x is a valid value.');
	t.ok(lastKnownPos.y !== undefined, 'mousepos.y is a valid value.');
});

test('Move the mouse.', function(t) 
{
	t.plan(3);
	lastKnownPos = robot.moveMouse(0, 0);
	t.ok(robot.moveMouse(100, 100), 'successfully moved the mouse.');
	currentPos = robot.getMousePos();
	t.ok(currentPos.x === 100, 'mousepos.x is correct.');
	t.ok(currentPos.y === 100, 'mousepos.y is correct.');
});


/*test('Click the mouse.', function(t) 
{
	t.plan(1);
	t.timeoutAfter(1000);
	
	detectMouse(function(e)
	{
		t.ok(e.button === 'left', 'mouse clicked.');
	});

	robot.mouseClick();
});*/
