var test = require('tape');
var robot = require('..');
var lastKnownPos, currentPos;

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
	t.plan(6);
	lastKnownPos = robot.moveMouse(0, 0);
	t.ok(robot.moveMouse(100, 100) === 1, 'successfully moved the mouse.');
	currentPos = robot.getMousePos();
	t.ok(currentPos.x === 100, 'mousepos.x is correct.');
	t.ok(currentPos.y === 100, 'mousepos.y is correct.');
	
	t.throws(function()
	{
		robot.moveMouse(0, 1, 2, 3);
	}, /Invalid number/, 'move mouse to (0, 1, 2, 3).');
	
	t.throws(function()
	{
		robot.moveMouse(0);
	}, /Invalid number/, 'move mouse to (0).');
	
	t.ok(robot.moveMouse("0", "0") === 1, 'move mouse to ("0", "0").');

});

test('Move the mouse smoothly.', function(t) 
{
	t.plan(6);
	lastKnownPos = robot.moveMouseSmooth(0, 0);
	t.ok(robot.moveMouseSmooth(100, 100) === 1, 'successfully moved the mouse.');
	currentPos = robot.getMousePos();
	t.ok(currentPos.x === 100, 'mousepos.x is correct.');
	t.ok(currentPos.y === 100, 'mousepos.y is correct.');
	
	t.throws(function()
	{
		robot.moveMouseSmooth(0, 1, 2, 3);
	}, /Invalid number/, 'move mouse to (0, 1, 2, 3).');
	
	t.throws(function()
	{
		robot.moveMouseSmooth(0);
	}, /Invalid number/, 'move mouse to (0).');
	
	t.ok(robot.moveMouseSmooth("0", "0") === 1, 'move mouse to ("0", "0").');

});

test('Click the mouse.', function(t) 
{
	t.plan(8);
	t.ok(robot.mouseClick(), 'click the mouse (no button specified).');
	t.ok(robot.mouseClick("left") === 1, 'click the left mouse button.');
	t.ok(robot.mouseClick("middle") === 1, 'click the middle mouse button.');
	t.ok(robot.mouseClick("right") === 1, 'click the right mouse button.');
	
	t.ok(robot.mouseClick("left", 1), 'double click the left mouse button.');
	
	t.throws(function()
	{
		robot.mouseClick("party");
	}, /Invalid mouse/, 'click an incorrect mouse button (party).');
	
	t.throws(function()
	{
		robot.mouseClick("0");
	}, /Invalid mouse/, 'click an incorrect mouse button (0).');
	
	t.throws(function()
	{
		robot.mouseClick("left", 0, "test");
	}, /Invalid number/, 'click the mouse with an extra argument.');
	
});

test('Drag the mouse.', function(t) 
{
	t.plan(4);
	
	t.ok(robot.dragMouse(5, 5) === 1, 'successfully dragged the mouse.');
	
	t.throws(function()
	{
		robot.dragMouse(0);
	}, /Invalid number/, 'drag mouse to (0).');
	
	t.throws(function()
	{
		robot.dragMouse(1, 1, "left", 5);
	}, /Invalid number/, 'drag mouse with extra argument.');
	
	t.throws(function()
	{
		robot.dragMouse(2, 2, "party");
	}, /Invalid mouse/, 'drag an incorrect mouse button (party).');
	
});

test('Mouse scroll.', function(t)
{
	t.plan(9);
	t.ok(lastKnownPos = robot.getMousePos(), 'successfully retrieved mouse position.');
	t.ok(robot.mouseClick() === 1, 'click the mouse (no button specified).');
	t.ok(robot.scrollMouse(0, 1 * 120) === 1, '1 scroll up.');
	t.ok(robot.scrollMouse(0, 20 * 120) === 1, '20 scrolls up.');
	t.ok(robot.scrollMouse(0, -5 * 120) === 1, '5 scrolls down.');
	t.ok(robot.scrollMouse(1 * 120, 0) === 1, '1 scroll right.');
	t.ok(robot.scrollMouse(20 * 120, 0) === 1, '20 scrolls right.');
	t.ok(robot.scrollMouse(-5 * 120, 0) === 1, '5 scrolls left.');
	t.ok(robot.scrollMouse(-5 * 120, -5 * 120) === 1, '5 scrolls left.');
});

test('Mouse Toggle', function(t)
{
	t.plan(2);
	t.ok(lastKnownPos = robot.getMousePos(), 'successfully retrieved mouse position.');
	t.ok(robot.mouseToggle('up', 'right') === 1, 'right click was pressed.');
});
