/* jshint esversion: 6 */
var robot = require('../..');
var targetFixture = require('../helpers/targetpractice');
var elements, target;
var originalTimeout;
const TARGET_COLOR = targetFixture.TARGET_COLOR;
const TARGET_LOGICAL_SIZE = 50;

describe('Integration/Screen', () => {
	beforeAll(() => {
		originalTimeout = jasmine.DEFAULT_TIMEOUT_INTERVAL;
		jasmine.DEFAULT_TIMEOUT_INTERVAL = 15000;
	});

	afterAll(() => {
		jasmine.DEFAULT_TIMEOUT_INTERVAL = originalTimeout;
	});

	beforeEach(() => {
		return targetFixture.start(robot).then(session => {
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
