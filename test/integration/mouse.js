/* jshint esversion: 6 */
var robot = require('../..');
var targetFixture = require('../helpers/targetpractice');

robot.setMouseDelay(100);

var target, elements;
var originalTimeout;

describe('Integration/Mouse', () => {
	beforeAll(() => {
		originalTimeout = jasmine.DEFAULT_TIMEOUT_INTERVAL;
		jasmine.DEFAULT_TIMEOUT_INTERVAL = 15000;
	});

	afterAll(() => {
		jasmine.DEFAULT_TIMEOUT_INTERVAL = originalTimeout;
	});

	beforeEach(() => {
		return targetFixture.start(robot, { interactive: true }).then(session => {
			target = session;
			elements = session.elements;
		});
	});

	afterEach(() => {
		const currentTarget = target;
		target = null;
		elements = null;
		return currentTarget ? currentTarget.stop() : Promise.resolve();
	});

	it('clicks', done => {
		// Alright we got a click event, did we click the button we wanted?
		target.once('click', function(e)
		{
			expect(e.id).toEqual('button_1');
			expect(e.type).toEqual('click');
			done();
		});

		// For this test we want a button.
		var button_1 = elements.button_1;
		// Click it!
		robot.moveMouse(button_1.x, button_1.y);
		robot.mouseClick();
	});

	it('scrolls vertically', done => {
		target.once('scroll', element => {
			expect(element.id).toEqual('textarea_1');
			expect(element.scroll_y).toBeGreaterThan(0);
			expect(element.scroll_x).toEqual(0);
			done();
		});

		var textarea_1 = elements.textarea_1;
		robot.moveMouse(textarea_1.x, textarea_1.y);
		robot.mouseClick();
		robot.scrollMouse(0, -10);
	});

	it('scrolls horizontally', done => {
		target.once('scroll', element => {
			expect(element.id).toEqual('textarea_1');
			expect(element.scroll_x).toBeGreaterThan(0);
			expect(element.scroll_y).toEqual(0);
			done();
		});

		var textarea_1 = elements.textarea_1;
		robot.moveMouse(textarea_1.x, textarea_1.y);
		robot.mouseClick();
		robot.scrollMouse(-10, 0);
	});
});
