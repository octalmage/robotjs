/* jshint esversion: 6 */
var robot = require('../..');
var targetFixture = require('../helpers/targetpractice');

robot.setMouseDelay(100);
robot.setKeyboardDelay(100);

var target, elements;
var originalTimeout;
const macOSIt = process.platform === 'darwin' ? it : xit;
const selectAllModifier = process.platform === 'darwin' ? 'command' : 'control';
const TYPING_TIMEOUT_MS = 5000;

function expectNextTypedText(expected, done, next) {
	let lastText;
	let timer;
	const handleType = element => {
		if (element.id !== 'input_1') {
			return;
		}

		lastText = element.text;
		if (lastText !== expected) {
			return;
		}

		clearTimeout(timer);
		target.removeListener('type', handleType);
		if (next) {
			next();
		} else {
			done();
		}
	};

	timer = setTimeout(() => {
		target.removeListener('type', handleType);
		done.fail(new Error(
			'Timed out after ' + TYPING_TIMEOUT_MS + 'ms waiting for input_1 to contain ' +
			JSON.stringify(expected) + '; last text was ' + JSON.stringify(lastText) + '.'
		));
	}, TYPING_TIMEOUT_MS);
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

	it('types', done => {
		const stringToType = 'hello world';
		// Target Practice emits every input state; wait for the final value.
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

	it('replaces selected input with the platform select-all shortcut', done => {

		const initial = 'initial content';
		const replacement = 'replacement content';
		expectNextTypedText(initial, done, () => {
			expectNextTypedText(replacement, done);
			robot.keyTap('a', selectAllModifier);
			// Pace the replacement so a busy runner does not drop queued text
			// events while processing the shortcut.
			robot.typeStringDelayed(replacement, 600);
		});

		const input_1 = elements.input_1;
		robot.moveMouse(input_1.x, input_1.y);
		robot.mouseClick();
		robot.typeString(initial);
	});

	macOSIt('types a non-ASCII character with unicodeTap on macOS', done => {

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
