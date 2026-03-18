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
  it('Tap a Unicode character.', function() {
    expect(robot.unicodeTap("r".charCodeAt(0))).toEqual(1);
    expect(robot.unicodeTap("ά".charCodeAt(0))).toEqual(1);
    expect(robot.unicodeTap("ö".charCodeAt(0))).toEqual(1);
    expect(robot.unicodeTap("ち".charCodeAt(0))).toEqual(1);
    expect(robot.unicodeTap("嗨".charCodeAt(0))).toEqual(1);
    expect(robot.unicodeTap("ఝ".charCodeAt(0))).toEqual(1);

    expect(function()
    {
      robot.unicodeTap();
    }).toThrowError(/Invalid character typed./);
  });

  it('Test Key Toggle.', function() {
    expect(robot.keyToggle("a", "down")).toEqual(1);
    expect(robot.keyToggle("a", "up")).toEqual(1);

    expect(function()
    {
      robot.keyToggle("ά", "down");
    }).toThrowError(/Invalid key code specified./);

    expect(function()
    {
      robot.keyToggle("嗨", "down");
    }).toThrowError(/Invalid key code specified./);
  });

  it('Type Ctrl+Shift+RightArrow.', function() {
    var modifiers = [];
    modifiers.push('shift');
    modifiers.push('control');

    expect(robot.keyToggle("right", "down", modifiers)).toEqual(1);
    expect(robot.keyToggle("right", "up", modifiers)).toEqual(1);
  });

  it('Type a string.', function() {
    expect(robot.typeString("Typed rάöち嗨ఝ 1")).toEqual(1);

    expect(function()
    {
      robot.typeString();
    }).toThrowError(/Invalid number of arguments./);
  });

  it('Type a string with delay.', function() {
    // 10 characters per minute -> 3 seconds to write the whole sentence here.
    expect(robot.typeStringDelayed("Typed rάöち嗨ఝ with delay 1", 600)).toEqual(1);

    expect(function()
    {
      robot.typeStringDelayed();
    }).toThrowError(/Invalid number of arguments./);
  });
});
