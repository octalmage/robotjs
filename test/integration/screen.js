/* jshint esversion: 6 */
var test = require('tape');
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');
var elements;

robot.setMouseDelay(100);

test('Test clicking.', { timeout: 10000 }, function(t)
{
	t.plan(1);

	// Start the UI.
	var target = targetpractice.start();

	// Wait for the list of elements.
	target.on('elements', function(elements)
	{
		// For this test we want a button.
		// setTimeout(() => {
			var color_1 = elements.color_1;
			const color = robot.getPixelColor(color_1.x, color_1.y);
			t.equal(color, "c0ff33", 'Color is what we expected.');
		// }, 100);
	});

	// Close the UI.
	t.on("end", function()
	{
		targetpractice.stop();
	});
});
