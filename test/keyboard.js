var test = require('tape');
var robot = require('..');
var stdin = process.openStdin();

test('Type hello world.', function(t) 
{
	robot.setMouseDelay(200);
	
	var screenSize = robot.getScreenSize();
	
	robot.moveMouse(screenSize.width / 2, screenSize.height / 2);
	robot.mouseClick();
	
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