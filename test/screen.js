var test = require('tape');
var fs = require('fs');
var robot = require('..');
var pixelColor, screenSize;

test('Get pixel color.', function(t) 
{
	t.plan(4);
	t.ok(pixelColor = robot.getPixelColor(5, 5), 'successfully got pixel color.');
	t.ok(pixelColor !== undefined, 'pixelColor is a valid value.');
	t.ok(pixelColor.length === 6, 'pixelColor is the correct length.');
	t.ok(/^[0-9A-F]{6}$/i.test(pixelColor), "pixelColor is valid hex.");
});

test('Get screen size.', function(t) 
{
	t.plan(3);
	t.ok(screenSize = robot.getScreenSize(), 'successfully got the screen size.');
	t.ok(screenSize.width !== undefined, 'screenSize.width is a valid value.');
	t.ok(screenSize.height !== undefined, 'screenSize.height is a valid value.');
});


test('Get screen shoot.', function(t)
{
	t.plan(3);
	t.ok(screenSize = robot.getScreenshoot('test.png'), 'successfully got the screen shoot.');
	t.ok(fs.openSync('test.png', 'r'), 'Check saved screenshoot.');
	t.ok(fs.unlinkSync('test.png') == undefined, 'Remove saved screenhoot.');
});
