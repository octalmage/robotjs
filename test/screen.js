var robot = require('..');
var pixelColor, screenSize;

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
});
