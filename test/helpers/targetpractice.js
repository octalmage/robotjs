/* jshint esversion: 8 */
var targetpractice = require('targetpractice');

const TARGET_COLOR = 'c0ff33';
const TRANSITION_TIMEOUT_MS = 5000;
const POLL_INTERVAL_MS = 25;
const INTERACTION_POLL_INTERVAL_MS = 100;

function waitForMarker(robot, point) {
	const deadline = Date.now() + TRANSITION_TIMEOUT_MS;
	let lastColor;

	return new Promise((resolve, reject) => {
		function poll() {
			try {
				lastColor = robot.getPixelColor(point.x, point.y);
			} catch (error) {
				reject(new Error(
					'Could not sample the Target Practice marker at (' + point.x + ', ' + point.y +
					'): ' + error.message
				));
				return;
			}

			if (lastColor === TARGET_COLOR) {
				resolve();
				return;
			}

			if (Date.now() >= deadline) {
				reject(new Error(
					'Timed out after ' + TRANSITION_TIMEOUT_MS +
					'ms waiting for the Target Practice marker at (' + point.x + ', ' + point.y +
					') to appear; last color was ' + lastColor + '.'
				));
				return;
			}

			setTimeout(poll, POLL_INTERVAL_MS);
		}

		poll();
	});
}

function waitForInteraction(robot, target, point) {
	const deadline = Date.now() + TRANSITION_TIMEOUT_MS;

	return new Promise((resolve, reject) => {
		let retryTimer;

		function cleanup() {
			clearTimeout(retryTimer);
			target.removeListener('click', handleClick);
		}

		function handleClick(event) {
			if (event.id !== 'button_1') {
				return;
			}

			cleanup();
			resolve();
		}

		function attempt() {
			if (Date.now() >= deadline) {
				cleanup();
				reject(new Error(
					'Timed out after ' + TRANSITION_TIMEOUT_MS +
					'ms waiting for Target Practice to accept mouse input.'
				));
				return;
			}

			try {
				robot.moveMouse(point.x, point.y);
				robot.mouseClick();
			} catch (error) {
				cleanup();
				reject(error);
				return;
			}

			retryTimer = setTimeout(attempt, INTERACTION_POLL_INTERVAL_MS);
		}

		target.on('click', handleClick);
		attempt();
	});
}

async function start(robot, options) {
	options = options || {};

	let target;
	try {
		target = await targetpractice.start();
		const elements = target.elements;

		if (!elements.color_1) {
			throw new Error('Target Practice did not report its color marker.');
		}
		if (options.interactive && !elements.button_1) {
			throw new Error('Target Practice did not report its interaction probe.');
		}

		await waitForMarker(robot, elements.color_1);
		if (options.interactive) {
			await waitForInteraction(robot, target, elements.button_1);
		}

		return target;
	} catch (error) {
		if (target) {
			try {
				await target.stop();
			} catch (stopError) {
				error.message += '\nshutdown error: ' + stopError.message;
			}
		}
		throw error;
	}
}

module.exports = {
	TARGET_COLOR: TARGET_COLOR,
	start: start
};
