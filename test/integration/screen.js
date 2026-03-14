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

	it('reads a pixel color', () => {
		const color_1 = elements.color_1;
		const color = robot.getPixelColor(color_1.x, color_1.y);
		expect(color).toEqual('c0ff33');
	});

	it('captures the full screen with the same top-left origin as getPixelColor', (done) => {
		try {
			const capture = robot.screen.capture();
			const expectedTopLeft = {
				x: Math.round((elements.color_1.x - 25) * capture.scaleX),
				y: Math.round((elements.color_1.y - 25) * capture.scaleY)
			};
			const probeCenter = {
				x: Math.round(elements.color_1.x * capture.scaleX),
				y: Math.round(elements.color_1.y * capture.scaleY)
			};

			expect(robot.getPixelColor(elements.color_1.x, elements.color_1.y)).toEqual('c0ff33');
			expect(capture.findColor('c0ff33')).toEqual(expectedTopLeft);
			expect(capture.colorAt(probeCenter.x, probeCenter.y)).toEqual('c0ff33');

			done();
		} catch (error) {
			done.fail(error);
		}

	});
});
