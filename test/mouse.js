var test = require('tape');
var robot = require('..');
var lastKnownPos;

test('get the initial mouse position', function(t) {
  t.plan(3);
  t.ok(lastKnownPos = robot.getMousePos(), 'successfully retrieved mouse position');
  t.ok(lastKnownPos.x !== undefined, 'mousepos.x is a valid value');
  t.ok(lastKnownPos.y !== undefined, 'mousepos.y is a valid value');
});