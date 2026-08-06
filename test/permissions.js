const robot = require('../index.js');

describe('Permissions', () => {
	it('Reports permission status without prompting.', () => {
		const accessibility = robot.getAccessibilityPermission();
		const screenCapture = robot.getScreenCapturePermission();

		if (process.platform === 'darwin') {
			expect(typeof accessibility).toBe('boolean');
			expect(typeof screenCapture).toBe('boolean');
		} else {
			expect(accessibility).toBeNull();
			expect(screenCapture).toBeNull();
		}
	});

	it('Exposes explicit permission request functions.', () => {
		expect(typeof robot.requestAccessibilityPermission).toBe('function');
		expect(typeof robot.requestScreenCapturePermission).toBe('function');

		if (process.platform !== 'darwin') {
			expect(robot.requestAccessibilityPermission()).toBeNull();
			expect(robot.requestScreenCapturePermission()).toBeNull();
		}
	});
});
