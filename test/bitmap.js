var test = require('tape');
var robot = require('..');

var params =
{
	'width': 'number',
	'height': 'number',
	'byteWidth': 'number',
	'bitsPerPixel': 'number',
	'bytesPerPixel': 'number',
	'image': 'object'
};

test('Get a bitmap.', function(t)
{
	t.plan(1);
	t.ok(robot.screen.capture(), 'got a bitmap.');
});

test('Get a bitmap and check the parameters.', function(t)
{
	t.plan(6);
	var img = robot.screen.capture();

	for (var x in params)
	{
		t.equal(typeof img[x], params[x], 'make sure ' + x  + ' is a ' + params[x] + '.');
	}
});

test('Get a bitmap of a specific size.', function(t)
{
	var size = 10;
	t.plan(2);
	var img = robot.screen.capture(0, 0, size, size);

	t.equals(img.height, size, 'make sure image is expected height.');
	t.equals(img.width, size, 'make sure image is expected width.');
});

test('Get a bitmap and make sure the colorAt works as expected.', function(t)
{
	t.plan(7);
	var img = robot.screen.capture();
	var hex = img.colorAt(0, 0);

	t.ok(/^[0-9A-F]{6}$/i.test(hex), "colorAt returned valid hex.");

	var screenSize = robot.getScreenSize();

	t.throws(function()
	{
		img.colorAt(0, screenSize.height);
	}, /are outside the bitmap/, 'colorAt (0, screen.height) threw an error.');

	t.doesNotThrow(function()
	{
		img.colorAt(0, screenSize.height-1);
	}, /are outside the bitmap/, 'colorAt (0, screen.height-1) did not throw an error.');

	t.throws(function()
	{
		img.colorAt(screenSize.width, 0);
	}, /are outside the bitmap/, 'colorAt (screen.width, 0) threw an error.');

	t.doesNotThrow(function()
	{
		img.colorAt(screenSize.width-1, 0);
	}, /are outside the bitmap/, 'colorAt (screen.width-1, 0) did not throw an error.');

	t.throws(function()
	{
		img.colorAt(9999999999999, 0);
	}, /are outside the bitmap/, 'colorAt (9999999999999, 0) threw an error.');

	// Regression test for https://github.com/octalmage/robotjs/commit/c41f38217fd73f59e6ca63015b51565cd1e7cfb7
	t.throws(function()
	{
		img.colorAt(0, 9999999999999);
	}, /are outside the bitmap/, 'colorAt (0, 9999999999999) threw an error.');
});
