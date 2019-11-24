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

	it('reads a pixel color', () => {
		var color_1 = elements.color_1;
		const color = robot.getPixelColor(color_1.x, color_1.y);
		expect(color).toEqual('c0ff33');
	});
});
