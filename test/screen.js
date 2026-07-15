var robot = require('..');
var displays, pixelColor, screenSize;

describe('Screen', () => {
  it('Get pixel color.', function()
  {
    expect(pixelColor = robot.getPixelColor(5, 5)).toBeTruthy();
    expect(pixelColor !== undefined).toBeTruthy();
    expect(pixelColor.length === 6).toBeTruthy();
    expect(/^[0-9A-F]{6}$/i.test(pixelColor)).toBeTruthy();

    expect(function()
    {
      robot.getPixelColor(9999999999999, 9999999999999);
    }).toThrowError(/outside the main screen/);

    expect(function()
    {
      robot.getPixelColor(-1, -1);
    }).toThrowError(/must be non-negative/);

    expect(function()
    {
      robot.getPixelColor(0);
    }).toThrowError(/Invalid number/);

    expect(function()
    {
      robot.getPixelColor(1, 2, 3);
    }).toThrowError(/Invalid number/);
  });

  it('Get screen size.', function()
  {
    expect(screenSize = robot.getScreenSize()).toBeTruthy();
    expect(screenSize.width !== undefined).toBeTruthy();
    expect(screenSize.height !== undefined).toBeTruthy();
  });

  it('Get displays.', function()
  {
    expect(displays = robot.getDisplays()).toBeTruthy();
    expect(Array.isArray(displays)).toBeTruthy();
    expect(displays.length > 0).toBeTruthy();
    expect(displays[0].id !== undefined).toBeTruthy();
    expect(displays[0].x !== undefined).toBeTruthy();
    expect(displays[0].y !== undefined).toBeTruthy();
    expect(displays[0].width !== undefined).toBeTruthy();
    expect(displays[0].height !== undefined).toBeTruthy();
    expect(displays[0].isMain !== undefined).toBeTruthy();
  });

  it('Captures pixels from a non-main display at its signed origin.', function()
  {
    var secondaryDisplays = robot.getDisplays().filter(function(display)
    {
      return !display.isMain;
    });
    var display, captureWidth, captureHeight, capture;

    if (secondaryDisplays.length === 0)
    {
      pending('Screen capture on a non-main display requires at least two active displays.');
    }

    display = secondaryDisplays.find(function(candidate)
    {
      return candidate.x < 0 || candidate.y < 0;
    }) || secondaryDisplays[0];
    captureWidth = Math.min(8, display.width);
    captureHeight = Math.min(8, display.height);
    capture = robot.screen.capture(display.x, display.y, captureWidth, captureHeight);

    expect(capture.screenX).toEqual(display.x);
    expect(capture.screenY).toEqual(display.y);
    expect(capture.scaleX).toBeGreaterThan(0);
    expect(capture.scaleY).toBeGreaterThan(0);
    expect(capture.scaleX).toBeCloseTo(capture.width / captureWidth, 10);
    expect(capture.scaleY).toBeCloseTo(capture.height / captureHeight, 10);
    expect(capture.colorAt(0, 0)).toMatch(/^[0-9A-F]{6}$/i);
  });
});
