/* jshint esversion: 6 */
var test = require('tape');
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');
var os = require('os');
var elements;

robot.setMouseDelay(100);

test('Test clicking.', { timeout: 10000 }, function(t)
{
	t.plan(2);

	// Start the UI.
	var target = targetpractice.start();

	// Wait for the list of elements.
	target.once('elements', function(elements)
	{
		// For this test we want a button.
		var button_1 = elements.button_1;

		// Click it!
		robot.moveMouse(button_1.x, button_1.y);
		robot.mouseClick();
	});

	// Alright we got a click event, did we click the button we wanted?
	target.once('click', function(e)
	{
		t.equal(e.id, "button_1", 'Confirm button_1 was clicked.');
		t.equal(e.type, "click", 'Confirm event was a click.');
	});

	// Close the UI.
	t.once('end', function()
	{
		targetpractice.stop();
	});
});

test('Test typing.', { timeout: 10000 }, function(t)
{
	t.plan(2);

	var target = targetpractice.start();
	var stringToType = "hello world";

	target.on('elements', function(elements)
	{
		var input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(stringToType);
	});

	// Currently Target Practice waits for the "user" to finish typing before sending the event.
	target.once('type', function(element)
	{
		t.equal(element.id, "input_1", 'Confirm input_1 was used.');
		t.equal(element.text, stringToType, `Confirm that ${stringToType} was typed.`);
	});

	t.once('end', function()
	{
		targetpractice.stop();
	});
});

test('Test scrolling.', { timeout: 10000 }, function(t)
{
	t.plan(2);

	var target = targetpractice.start();

	target.once('elements', function(elements)
	{
		var textarea_1 = elements.textarea_1;
		robot.moveMouse(textarea_1.x, textarea_1.y);
		robot.mouseClick();
		robot.scrollMouse(0, -10);
	});

	target.once('scroll', function(element)
	{
		/**
		 *  TODO: This is gross! The scroll distance is different for each OS. I want
		 *  to look into this further, but at least these numbers are consistent.
		 */
		let expectedScroll;
		switch(os.platform()) {
			case 'linux':
				expectedScroll = 180;
				break;
			case 'win32':
				expectedScroll = 8;
				break;
			default:
				expectedScroll = 10;
		}
		t.equal(element.id, 'textarea_1', 'Confirm textarea_1 was used.');
		t.equal(element.scroll_y, expectedScroll, 'Confirm scroll to 10.');
	});

	t.once('end', function()
	{
		targetpractice.stop();
	});
});
