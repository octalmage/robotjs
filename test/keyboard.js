var test = require('tape');
var robot = require('..');
var os = require('os');

//TODO: Need tests for keyToggle, typeString, typeStringDelayed, and setKeyboardDelay.

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
	t.plan(6);
	t.ok(robot.utf32Tap("r".charCodeAt(0)) === 1, 'successfully tapped "r".');
	t.ok(robot.utf32Tap("ά".charCodeAt(0)) === 1, 'successfully tapped "ά".');
	t.ok(robot.utf32Tap("ö".charCodeAt(0)) === 1, 'successfully tapped "ö".');
	t.ok(robot.utf32Tap("ち".charCodeAt(0)) === 1, 'successfully tapped "ち".');
	t.ok(robot.utf32Tap("嗨".charCodeAt(0)) === 1, 'successfully tapped "嗨".');

	t.throws(function()
	{
		robot.utf32Tap();
	}, /Invalid character typed./, 'tap nothing.');
});