/* jshint esversion: 6 */
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');

robot.setMouseDelay(100);

var target, elements;
var originalTimeout;

describe('Integration/Keyboard', () => {
	beforeAll(() => {
		originalTimeout = jasmine.DEFAULT_TIMEOUT_INTERVAL;
		jasmine.DEFAULT_TIMEOUT_INTERVAL = 15000;
	});

	afterAll(() => {
		jasmine.DEFAULT_TIMEOUT_INTERVAL = originalTimeout;
	});

	beforeEach(done => {
		target = targetpractice.start();
		target.once('error', done.fail);
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
		const handleType = element => {
			expect(element.id).toEqual('input_1');
			if (element.text === stringToType) {
				target.removeListener('type', handleType);
				done();
			}
		};
		target.on('type', handleType);

		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(stringToType);
	});
});
