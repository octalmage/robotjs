var robot = require('..');
var os = require('os');

// TODO: Need tests for keyToggle, typeString, typeStringDelayed, and setKeyboardDelay.

describe('Keyboard', () => {
  it('Tap a key.', function() {
    expect(() => robot.keyTap('a')).not.toThrow();
    expect(() => robot.keyTap('a', 'control')).not.toThrow();
    expect(() => robot.keyTap()).toThrowError(/Invalid number/);
  });

  // This it won't fail if there's an issue, but it will help you identify an issue if ran locally.
  it('Tap all keys.', function()
  {
    var chars = 'abcdefghijklmnopqrstuvwxyz1234567890,./;\'[]\\'.split('');

    for (var x in chars)
    {
      expect(() => robot.keyTap(chars[x])).not.toThrow();
    }
  });

  // This it won't fail if there's an issue, but it will help you identify an issue if ran locally.
  it('Tap all numpad keys.', function()
  {
    var nums = '0123456789'.split('');

    for (var x in nums)
    {
      if (os.platform() === 'linux')
      {
        expect(() => robot.keyTap('numpad_' + nums[x])).toThrowError(/Invalid key code/);
      }
      else
      {
        expect(() => robot.keyTap('numpad_' + nums[x])).not.toThrow();
      }
    }
  });
});

test('Tap a Unicode character.', function(t)
{
	t.plan(7);
	t.ok(robot.unicodeTap("r".charCodeAt(0)) === 1, 'successfully tapped "r".');
	t.ok(robot.unicodeTap("ά".charCodeAt(0)) === 1, 'successfully tapped "ά".');
	t.ok(robot.unicodeTap("ö".charCodeAt(0)) === 1, 'successfully tapped "ö".');
	t.ok(robot.unicodeTap("ち".charCodeAt(0)) === 1, 'successfully tapped "ち".');
	t.ok(robot.unicodeTap("嗨".charCodeAt(0)) === 1, 'successfully tapped "嗨".');
	t.ok(robot.unicodeTap("ఝ".charCodeAt(0)) === 1, 'successfully tapped "ఝ".');

	t.throws(function()
	{
		robot.unicodeTap();
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
