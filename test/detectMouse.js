module.exports = function(callback)
{

	var stdin = process.openStdin();

	stdin.setRawMode(true);

	stdin.resume();

	//Enable mouse.
	process.stdout.write("\x1b[?1000h");
	process.stdout.write("\x1b[?1002h");

	stdin.on("data", function(d)
	{
		var buttons, event, eventData;
		eventData = {};
		buttons = ['left', 'middle', 'right'];

		if (d[0] === 0x3)
		{
			quit();
		}

		if (d[0] === 0x1b && d[1] === 0x5b && d[2] === 0x4d)
		{
			switch (d[3] & 0x60)
			{
				case 0x20:
					if ((d[3] & 0x3) < 0x3)
					{
						event = 'mousedown';
						eventData.button = buttons[d[3] & 0x3];
					}
					else
					{
						event = 'mouseup';
					}
					break;
				case 0x40:
					event = 'drag';
					if ((d[3] & 0x3) < 0x3)
					{
						eventData.button = buttons[d[3] & 0x3];
					}
					break;
				case 0x60:
					event = 'wheel';
					if (d[3] & 0x1)
					{
						eventData.direction = 'down';
					}
					else
					{
						eventData.direction = 'up';
					}
			}
			eventData.shift = (d[3] & 0x4) > 0;
			eventData.x = d[4] - 32;
			eventData.y = d[5] - 32;
			
			//Disable mouse.
			process.stdout.write("\x1b[?1000l");
			process.stdout.write("\x1b[?1002l");
			
			process.stdin.setRawMode(false);
			stdin.pause();
			
			callback(eventData);
		}
	});
};
