var test = require('tape');
var robot = require('..');

//TODO: Need tests for keyToggle, typeString, typeStringDelayed, and setKeyboardDelay.

test('Tap a key.', function(t) 
{
	t.plan(4);
	t.ok(robot.keyTap("a") === 1, 'successfully tapped "a".');
	t.ok(robot.keyTap("a", "control") === 1, 'successfully tapped "ctrl+a".');
	
	t.throws(function()
	{
		robot.keyTap();
	}, /Invalid number/, 'tap nothing.');
	
	t.throws(function()
	{
		robot.keyTap();
	}, /Invalid number/, 'tap "ctrl+a" with an extra argument.');
});