/* jshint esversion: 6 */
var spawn = require('child_process').spawn;
var EventEmitter = require('events').EventEmitter;
var os = require('os');
var path = require('path');
var electron = require('electron');

function makeLaunchError(message, stderr, code, signal) {
	var details = [];

	if (code !== undefined && code !== null) {
		details.push('exit code: ' + code);
	}

	if (signal) {
		details.push('signal: ' + signal);
	}

	if (stderr) {
		details.push('stderr:\n' + stderr.trim());
	}

	return new Error(message + (details.length ? '\n' + details.join('\n') : ''));
}

module.exports.start = function start() {
	var emitter = new EventEmitter();
	var stdoutBuffer = '';
	var stderrBuffer = '';
	var appDir = path.dirname(require.resolve('targetpractice/main.js'));
	var args = [];

	if (os.platform() === 'linux') {
		args.push('--no-sandbox', '--disable-setuid-sandbox');
	}

	args.push(appDir);

	var child = spawn(electron, args, {
		detached: false,
		stdio: ['ignore', 'pipe', 'pipe']
	});

	emitter.process = child;
	emitter.stopped = false;

	child.stdout.on('data', function(data) {
		stdoutBuffer += data.toString('utf8');
		var messages = stdoutBuffer.split(/\r?\n/);
		stdoutBuffer = messages.pop();

		for (var x in messages)
		{
			try
			{
				var msg = JSON.parse(messages[x]);
				emitter.emit(msg.event, msg.message);
			}
			catch (e)
			{
				process.stderr.write("Couldn't decode JSON:\n" + messages[x] + "\n");
			}
		}
	});

	child.stderr.on('data', function(data) {
		var chunk = data.toString('utf8');
		stderrBuffer += chunk;
		process.stderr.write(chunk);
	});

	child.on('error', function(err) {
		if (!emitter.stopped) {
			emitter.emit('error', err);
		}
	});

	child.on('exit', function(code, signal) {
		if (!emitter.stopped && code !== 0) {
			emitter.emit('error', makeLaunchError('Target Practice exited before the fixture loaded.', stderrBuffer, code, signal));
		}
	});

	return emitter;
};

module.exports.stop = function stop(targetpractice) {
	if (targetpractice && targetpractice.process)
	{
		targetpractice.stopped = true;

		if (targetpractice.process.stdin)
		{
			targetpractice.process.stdin.pause();
		}

		targetpractice.process.kill();
		targetpractice.process = null;
	}
};
