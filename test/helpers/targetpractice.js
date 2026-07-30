/* jshint esversion: 6 */
var targetpractice = require('targetpractice/index.js');

const TARGET_COLOR = 'c0ff33';
const TRANSITION_TIMEOUT_MS = 5000;
const POLL_INTERVAL_MS = 25;
const INTERACTION_POLL_INTERVAL_MS = 100;

var activeSession = null;

function waitForMarker(robot, point, visible) {
	const deadline = Date.now() + TRANSITION_TIMEOUT_MS;
	const transition = visible ? 'appear' : 'disappear';
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

			if ((lastColor === TARGET_COLOR) === visible) {
				resolve();
				return;
			}

			if (Date.now() >= deadline) {
				reject(new Error(
					'Timed out after ' + TRANSITION_TIMEOUT_MS +
					'ms waiting for the Target Practice marker at (' + point.x + ', ' + point.y +
					') to ' + transition + '; last color was ' + lastColor + '.'
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

function start(robot, options) {
	options = options || {};
	if (activeSession !== null) {
		return Promise.reject(new Error('Target Practice is already running.'));
	}

	let target;
	try {
		target = targetpractice.start();
	} catch (error) {
		return Promise.reject(error);
	}

	const session = {
		elements: null,
		target: target
	};
	activeSession = session;

	return new Promise((resolve, reject) => {
		const elementsTimer = setTimeout(() => {
			cleanup();
			reject(new Error(
				'Timed out after ' + TRANSITION_TIMEOUT_MS +
				'ms waiting for Target Practice to report its elements.'
			));
		}, TRANSITION_TIMEOUT_MS);

		function cleanup() {
			clearTimeout(elementsTimer);
			target.removeListener('elements', handleElements);
			target.removeListener('error', handleError);
		}

		function handleError(error) {
			cleanup();
			reject(error);
		}

		function handleElements(elements) {
			clearTimeout(elementsTimer);
			target.removeListener('elements', handleElements);
			session.elements = elements;

			if (!elements.color_1) {
				cleanup();
				reject(new Error('Target Practice did not report its color marker.'));
				return;
			}
			if (options.interactive && !elements.button_1) {
				cleanup();
				reject(new Error('Target Practice did not report its interaction probe.'));
				return;
			}

			let ready = waitForMarker(robot, elements.color_1, true);
			if (options.interactive) {
				ready = ready.then(() => waitForInteraction(robot, target, elements.button_1));
			}

			ready.then(() => {
				cleanup();
				resolve(session);
			}, error => {
				cleanup();
				reject(error);
			});
		}

		target.once('elements', handleElements);
		target.once('error', handleError);
	});
}

function stop(robot) {
	if (activeSession === null) {
		return Promise.resolve();
	}

	const session = activeSession;
	const marker = session.elements && session.elements.color_1;

	try {
		targetpractice.stop();
	} catch (error) {
		activeSession = null;
		return Promise.reject(error);
	}

	const closed = marker ? waitForMarker(robot, marker, false) : Promise.resolve();
	return closed.then(() => {
		activeSession = null;
	}, error => {
		activeSession = null;
		throw error;
	});
}

module.exports = {
	TARGET_COLOR: TARGET_COLOR,
	start: start,
	stop: stop
};
