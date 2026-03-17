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
    }).toThrowError(/outside the main screen/);

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
});
