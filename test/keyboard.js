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
