var test = require('tape');
var robot = require('..');
var stdin = process.openStdin();

test('Type hello world.', function(t) 
{
	t.plan(1);
	t.timeoutAfter(1000);
	
	stdin.resume();
	
	stdin.addListener("data", function(d) 
	{
    	t.ok(d.toString().trim() === "hello world", 'Got hello world.');
		stdin.removeAllListeners("data");
		process.stdin.pause();
	});
	
	robot.typeString("hello world");
	robot.keyTap("enter");
	
});