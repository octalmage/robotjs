var test = require('tape');
var robot = require('..');
var os = require('os');

// TODO: Need tests for setKeyboardDelay.

test('Tap a key.', function(t)
{
	t.plan(5);
	t.ok(robot.keyTap("a") === 1, 'successfully tapped "a".');
	t.ok(robot.keyTap("control") === 1, 'successfully tapped "ctrl".');
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

// This test won't fail if there's an issue, but it will help you identify an issue if ran locally.
test('Tap all keys.', function(t)
{
	var chars = 'abcdefghijklmnopqrstuvwxyz1234567890,./;\'[]\\'.split('');

	for (var x in chars)
	{
		t.ok(robot.keyTap(chars[x]), 'tap ' + chars[x] + '.');
	}

	t.end();
});

// This test won't fail if there's an issue, but it will help you identify an issue if ran locally.
test('Tap all numpad keys.', function(t)
{
	var nums = '0123456789'.split('');

	for (var x in nums)
	{
		if (os.platform() === 'linux')
		{
			/* jshint loopfunc:true */
			t.throws(function()
			{
				robot.keyTap('numpad_' + nums[x]);
			}, /Invalid key code/, 'tap ' + 'numpad_' + nums[x] + ' threw an error.');
		}
		else
		{
			t.ok(robot.keyTap('numpad_' + nums[x]), 'tap ' + 'numpad_' + nums[x] + '.');
		}
	}

	t.end();
});

test('Tap a UTF32 character.', function(t)
{
	t.plan(7);
	t.ok(robot.utf32Tap("r".charCodeAt(0)) === 1, 'successfully tapped "r".');
	t.ok(robot.utf32Tap("ά".charCodeAt(0)) === 1, 'successfully tapped "ά".');
	t.ok(robot.utf32Tap("ö".charCodeAt(0)) === 1, 'successfully tapped "ö".');
	t.ok(robot.utf32Tap("ち".charCodeAt(0)) === 1, 'successfully tapped "ち".');
	t.ok(robot.utf32Tap("嗨".charCodeAt(0)) === 1, 'successfully tapped "嗨".');
	t.ok(robot.utf32Tap("ఝ".charCodeAt(0)) === 1, 'successfully tapped "ఝ".');

	t.throws(function()
	{
		robot.utf32Tap();
	}, /Invalid character typed./, 'tap nothing.');
});

test('Test Key Toggle.', function(t)
{
	t.plan(4);

	t.ok(robot.keyToggle("a", "down") === 1, 'Successfully pressed a.');
	t.ok(robot.keyToggle("a", "up") === 1, 'Successfully released a.');

	t.throws(function()
	{
		t.ok(robot.keyToggle("ά", "down") === 1, 'Successfully pressed ά.');
		t.ok(robot.keyToggle("ά", "up") === 1, 'Successfully released ά.');
	}, /Invalid key code specified./, 'exception tapping ά.');

	t.throws(function()
	{
		t.ok(robot.keyToggle("嗨", "down") === 1, 'Successfully pressed 嗨.');
		t.ok(robot.keyToggle("嗨", "up") === 1, 'Successfully released 嗨.');
	}, /Invalid key code specified./, 'exception tapping 嗨.');
});

test('Type Ctrl+Shift+RightArrow.', function(t)
{
	t.plan(2);

	var modifiers = []
    modifiers.push('shift')
	modifiers.push('control')

	t.ok(robot.keyToggle("right", "down", modifiers) === 1, 'Successfully pressed Ctrl+Shift+RightArrow.');
	t.ok(robot.keyToggle("right", "up", modifiers) === 1, 'Successfully released Ctrl+Shift+RightArrow.');
});

test('Type a string.', function(t)
{
	t.plan(2);
	t.ok(robot.typeString("Typed rάöち嗨ఝ 1") === 1, 'successfully typed "Typed rάöち嗨ఝ 1".');

	t.throws(function()
	{
		t.ok(robot.typeString() === 1, 'Successfully typed nothing.');
	}, /Invalid number of arguments./, 'exception tapping nothing.');
});

test('Type a string with delay.', function(t)
{
	t.plan(2);

	// 10 characters per minute -> 3 seconds to write the whole sentence here.
	t.ok(robot.typeStringDelayed("Typed rάöち嗨ఝ with delay 1", 600) === 1, 'successfully typed with delay "Typed rάöち嗨ఝ with delay 1".');

	t.throws(function()
	{
		t.ok(robot.typeStringDelayed() === 1, 'Successfully typed nothing.');
	}, /Invalid number of arguments./, 'exception tapping nothing.');
});
