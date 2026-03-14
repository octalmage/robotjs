/* jshint esversion: 6 */
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');
var elements, target;
var originalTimeout;

describe('Integration/Screen', () => {
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

	it('reads a pixel color', (done) => {
		const maxDelay = 1000
		const expected = 'c0ff33'
		const color_1 = elements.color_1;
		const sleepTime = robot.getPixelColor(color_1.x, color_1.y) === expected ? 0
			: maxDelay

		setTimeout(() => {
			const color = robot.getPixelColor(color_1.x, color_1.y);
			expect(color).toEqual(expected);
			done()
		}, sleepTime)
	});
});
