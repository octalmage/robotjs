var robot = require('..');
var lastKnownPos, currentPos;

//Increase delay to help it reliability.
robot.setMouseDelay(100);

describe('Mouse', () => {
  it('Get the initial mouse position.', function()
  {
    expect(lastKnownPos = robot.getMousePos()).toBeTruthy();
    expect(lastKnownPos.x !== undefined).toBeTruthy();
    expect(lastKnownPos.y !== undefined).toBeTruthy();
  });

  it('Move the mouse.', function()
  {
    lastKnownPos = robot.moveMouse(0, 0);
    expect(robot.moveMouse(100, 100) === 1).toBeTruthy();
    currentPos = robot.getMousePos();
    expect(currentPos.x === 100).toBeTruthy();
    expect(currentPos.y === 100).toBeTruthy();

    expect(function()
    {
      robot.moveMouse(0, 1, 2, 3);
    }).toThrowError(/Invalid number/);

    expect(function()
    {
      robot.moveMouse(0);
    }).toThrowError(/Invalid number/);

    expect(robot.moveMouse("0", "0") === 1).toBeTruthy();

  });

  it('Move the mouse smoothly.', function()
  {
    lastKnownPos = robot.moveMouseSmooth(0, 0);
    expect(robot.moveMouseSmooth(100, 100) === 1).toBeTruthy();
    currentPos = robot.getMousePos();
    expect(currentPos.x).toEqual(100);
    expect(currentPos.y).toEqual(100);

    expect(function()
    {
      robot.moveMouseSmooth(0, 1, 2, 3);
    }).toThrowError(/Invalid number/);

    expect(function()
    {
      robot.moveMouseSmooth(0);
    }).toThrowError(/Invalid number/);

    expect(robot.moveMouseSmooth("0", "0") === 1).toBeTruthy();

  });

  it('Click the mouse.', function()
  {
    expect(robot.mouseClick()).toBeTruthy();
    expect(robot.mouseClick("left") === 1).toBeTruthy();
    expect(robot.mouseClick("middle") === 1).toBeTruthy();
    expect(robot.mouseClick("right") === 1).toBeTruthy();

    expect(robot.mouseClick("left", 1)).toBeTruthy();

    expect(function()
    {
      robot.mouseClick("party");
    }).toThrowError(/Invalid mouse/);

    expect(function()
    {
      robot.mouseClick("0");
    }).toThrowError(/Invalid mouse/);

    expect(function()
    {
      robot.mouseClick("left", 0, "it");
    }).toThrowError(/Invalid number/);

  });

  it('Drag the mouse.', function()
  {

    expect(robot.dragMouse(5, 5) === 1).toBeTruthy();

    expect(function()
    {
      robot.dragMouse(0);
    }).toThrowError(/Invalid number/);

    expect(function()
    {
      robot.dragMouse(1, 1, "left", 5);
    }).toThrowError(/Invalid number/);

    expect(function()
    {
      robot.dragMouse(2, 2, "party");
    }).toThrowError(/Invalid mouse/);

  });

  it('Mouse scroll.', function()
  {
    expect(lastKnownPos = robot.getMousePos()).toBeTruthy();
    expect(robot.mouseClick() === 1).toBeTruthy();
    expect(robot.scrollMouse(0, 1 * 120) === 1).toBeTruthy();
    expect(robot.scrollMouse(0, 20 * 120) === 1).toBeTruthy();
    expect(robot.scrollMouse(0, -5 * 120) === 1).toBeTruthy();
    expect(robot.scrollMouse(1 * 120, 0) === 1).toBeTruthy();
    expect(robot.scrollMouse(20 * 120, 0) === 1).toBeTruthy();
    expect(robot.scrollMouse(-5 * 120, 0) === 1).toBeTruthy();
    expect(robot.scrollMouse(-5 * 120, -5 * 120) === 1).toBeTruthy();
  });

  it('Mouse Toggle', function()
  {
    expect(lastKnownPos = robot.getMousePos()).toBeTruthy();
    expect(robot.mouseToggle('up', 'right') === 1).toBeTruthy();
  });
});
