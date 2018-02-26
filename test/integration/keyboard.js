/* jshint esversion: 6 */
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');
var os = require('os');

robot.setMouseDelay(100);

var target, elements;

describe('Integration/Keyboard', () => {
	beforeEach(done => {
		target = targetpractice.start();
		target.once('elements', message => {
			elements = message;
			done();
		});
	});

	afterEach(() => {
		targetpractice.stop();
		target = null;
	});

	it('types', done => {
		const stringToType = 'hello world';
		// Currently Target Practice waits for the "user" to finish typing before sending the event.
		target.once('type', element => {
			expect(element.id).toEqual('input_1');
			expect(element.text).toEqual(stringToType);
			done();
		});

		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(stringToType);
	});
});
