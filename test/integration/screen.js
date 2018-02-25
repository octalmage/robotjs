/* jshint esversion: 6 */
var test = require('tape');
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');
var elements;

robot.setMouseDelay(100);

test('Test reading the Screen.', { timeout: 10000 }, function(t)
{
	t.plan(1);

	// Start the UI.
	var target = targetpractice.start();

	// Wait for the list of elements.
	target.once('elements', function(elements)
	{
		var color_1 = elements.color_1;
		const color = robot.getPixelColor(color_1.x, color_1.y);
		t.equal(color, 'c0ff33', 'Color is what we expected.');
	});

	// Close the UI.
	t.once('end', function()
	{
		targetpractice.stop();
	});
});
