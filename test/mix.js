var test = require('tape');
var robot = require('..');
var os = require('os');


//Increase delay to help test reliability.
robot.setMouseDelay(100);

test('Send Ctrl+Shift+Click.', function(t)
{
    t.plan(5);

    t.ok(robot.keyToggle("control", "down") === 1, 'Successfully pressed Ctrl.');
    t.ok(robot.keyToggle("shift", "down") === 1, 'Successfully pressed Shift.');
    t.ok(robot.mouseClick(), 'Successfully clicked the default mouse button.');
    t.ok(robot.keyToggle("control", "up") === 1, 'Successfully released Ctrl.');
    t.ok(robot.keyToggle("shift", "up") === 1, 'Successfully released Shift.');
});

test('Send Ctrl+Shift+Click using modifiers.', function(t)
{
	t.plan(3);

	var modifiers = []
    modifiers.push('shift')

	t.ok(robot.keyToggle("control", "down", modifiers) === 1, 'Successfully pressed Ctrl+Shift.');
    t.ok(robot.mouseClick(), 'Successfully clicked the default mouse button.');
    t.ok(robot.keyToggle("control", "up", modifiers) === 1, 'Successfully released Ctrl+Shift.');
});