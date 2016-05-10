var robot = require("./index.js");


var bmp = robot.screen.capture(0, 0, 500, 500);

//bmp.save("./test.bmp");
//
// var start = process.hrtime();
//
// var elapsed_time = function(note)
// {
// 	var precision = 3; // 3 decimal places
// 	var elapsed = process.hrtime(start)[1] / 1000000; // divide by a million to get nano to milli
// 	console.log(process.hrtime(start)[0] + " s, " + elapsed.toFixed(precision) + " ms - " + note); // print message + time
// 	start = process.hrtime(); // reset the timer
// }
//
// for (var i = 1; i < 1000; i++)
// {
// 	testColorAt();
// }
//
// function testColorAt()
// {
//
// 	var bmp = robot.screen.capture(0,0, 500, 500);
// 	//console.log(bmp.width);
//     	//console.log(bmp.height);
//     	//
//     console.log(bmp)
// 	for (var x = 0; x < 1000; x++)
// 	{
// 		var hex = bmp.colorAt(2, 5);
// 	}
//
// 	elapsed_time("colorAt #" + i);
//
// }
