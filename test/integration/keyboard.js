/* jshint esversion: 6 */
var robot = require('../..');
var targetpractice = require('targetpractice/index.js');

robot.setMouseDelay(100);

var target, elements;
var originalTimeout;

function expectNextTypedText(expected, done, next) {
	const handleType = element => {
		if (element.id !== 'input_1') {
			return;
		}

		target.removeListener('type', handleType);
		expect(element.text).toEqual(expected);
		if (next) {
			next();
		} else {
			done();
		}
	};

	target.on('type', handleType);
}

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
		// Target Practice emits after the user has stopped typing.
		expectNextTypedText(stringToType, done);

		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(stringToType);
	});

	// Regression for https://github.com/octalmage/robotjs/pull/797
	it('types shifted symbols', done => {
		const stringToType = '!@#$%^&*()_+{}|:"<>?';
		expectNextTypedText(stringToType, done);

		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(stringToType);
	});

	it('replaces selected input with a command-modified key tap on macOS', done => {
		if (process.platform !== 'darwin') {
			pending('macOS only: verifies command-modified keyboard events.');
			return;
		}

		const initial = 'initial content';
		const replacement = 'replacement content';
		expectNextTypedText(initial, done, () => {
			expectNextTypedText(replacement, done);
			robot.keyTap('a', 'command');
			robot.typeString(replacement);
		});

		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(initial);
	});

	it('types a non-ASCII character with unicodeTap on macOS', done => {
		if (process.platform !== 'darwin') {
			pending('macOS only: verifies Unicode keyboard events.');
			return;
		}

		const marker = 'x';
		const character = '嗨';
		expectNextTypedText(marker, done, () => {
			expectNextTypedText(character, done);
			robot.keyTap('backspace');
			robot.unicodeTap(character.charCodeAt(0));
		});

		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(marker);
	});

	// Regression for https://github.com/octalmage/robotjs/issues/789
	// keyTap("tab") and other multi-character key names threw "Invalid key code specified".
	it('keyTap accepts multi-character key names', () => {
		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();

		const keys = ['tab', 'escape', 'space', 'enter', 'backspace', 'home', 'end'];
		for (let i = 0; i < 10; i++) {
			for (const key of keys) {
				expect(() => robot.keyTap(key)).not.toThrow();
			}
		}
	});
});
