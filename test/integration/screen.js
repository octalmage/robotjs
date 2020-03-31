/* jshint esversion: 6 */
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');
var elements, target;

describe('Integration/Screen', () => {
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

	it('reads a pixel color', (done) => {
		const maxDelay = 1000
		jasmine.DEFAULT_TIMEOUT_INTERVAL = maxDelay + 1000
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
