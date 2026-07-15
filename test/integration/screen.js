/* jshint esversion: 6 */
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');
var elements, target, readinessTimer;
var originalTimeout;
const TARGET_COLOR = 'c0ff33';
const TARGET_LOGICAL_SIZE = 50;
const TARGET_READY_TIMEOUT_MS = 5000;
const TARGET_POLL_INTERVAL_MS = 25;

function waitForTargetPixel(done) {
	const point = elements.color_1;
	const deadline = Date.now() + TARGET_READY_TIMEOUT_MS;
	let lastColor;

	function poll() {
		readinessTimer = null;

		try {
			lastColor = robot.getPixelColor(point.x, point.y);
		} catch (error) {
			done.fail(error);
			return;
		}

		if (lastColor === TARGET_COLOR) {
			done();
			return;
		}

		if (Date.now() >= deadline) {
			done.fail(new Error(
				'Timed out after ' + TARGET_READY_TIMEOUT_MS +
				'ms waiting for Target Practice pixel at (' + point.x + ', ' + point.y +
				') to be painted ' + TARGET_COLOR + '; last color was ' + lastColor + '.'
			));
			return;
		}

		readinessTimer = setTimeout(poll, TARGET_POLL_INTERVAL_MS);
	}

	poll();
}

describe('Integration/Screen', () => {
	beforeAll(() => {
		originalTimeout = jasmine.DEFAULT_TIMEOUT_INTERVAL;
		jasmine.DEFAULT_TIMEOUT_INTERVAL = 15000;
	});

	afterAll(() => {
		jasmine.DEFAULT_TIMEOUT_INTERVAL = originalTimeout;
	});

	beforeEach(done => {
		readinessTimer = null;
		target = targetpractice.start();
		target.once('elements', message => {
			elements = message;
			waitForTargetPixel(done);
		});
	});

	afterEach(() => {
		if (readinessTimer !== null) {
			clearTimeout(readinessTimer);
			readinessTimer = null;
		}
		targetpractice.stop();
		target = null;
		elements = null;
	});

	it('reads a pixel color', () => {
		const color_1 = elements.color_1;
		const color = robot.getPixelColor(color_1.x, color_1.y);
		expect(color).toEqual(TARGET_COLOR);
	});

	it('captures the full screen with the same top-left origin as getPixelColor', (done) => {
		try {
			const capture = robot.screen.capture();
			const expectedTopLeft = {
				x: Math.round((elements.color_1.x - (TARGET_LOGICAL_SIZE / 2)) * capture.scaleX),
				y: Math.round((elements.color_1.y - (TARGET_LOGICAL_SIZE / 2)) * capture.scaleY)
			};
			const probeCenter = {
				x: Math.round(elements.color_1.x * capture.scaleX),
				y: Math.round(elements.color_1.y * capture.scaleY)
			};
			const targetDimensions = {
				width: Math.round(TARGET_LOGICAL_SIZE * capture.scaleX),
				height: Math.round(TARGET_LOGICAL_SIZE * capture.scaleY)
			};
			const match = capture.findColor(TARGET_COLOR);

			expect(robot.getPixelColor(elements.color_1.x, elements.color_1.y)).toEqual(TARGET_COLOR);
			expect(match).toEqual(expectedTopLeft);
			expect(capture.colorAt(probeCenter.x, probeCenter.y)).toEqual(TARGET_COLOR);
			expect(capture.toScreenPoint(match, targetDimensions)).toEqual({
				x: elements.color_1.x,
				y: elements.color_1.y
			});

			done();
		} catch (error) {
			done.fail(error);
		}
	});
});
