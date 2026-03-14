const robot = require('..');
const makeBitmap = require('./helpers/images').makeBitmap;

describe('Bitmap', () => {
  const params = {
		'width': 'number',
		'height': 'number',
		'byteWidth': 'number',
		'bitsPerPixel': 'number',
		'bytesPerPixel': 'number',
		'image': 'object'
	};

	it('Get a bitmap and check the parameters.', function() {
		const img = robot.screen.capture();

		for (const x in params)
		{
			expect(typeof img[x]).toEqual(params[x]);
		}
	});

	it('Get a bitmap of a specific size.', function()
	{
		const size = 10;
		const img = robot.screen.capture(0, 0, size, size);

		// Support for higher density screens.
		const multi = img.width / size;
		const scaledSize = size * multi;
		expect(img.height).toEqual(scaledSize);
		expect(img.width).toEqual(scaledSize);
	});

	it('Get a bitmap and make sure the colorAt works as expected.', function()
	{
		const img = robot.screen.capture();
		const hex = img.colorAt(0, 0);

		// t.ok(.it(hex), "colorAt returned valid hex.");
		expect(hex).toMatch(/^[0-9A-F]{6}$/i);

		const screenSize = robot.getScreenSize();
		let width = screenSize.width;
		let height = screenSize.height;

		// Support for higher density screens.
		const multi = img.width / width;
		width = width * multi;
		height = height * multi;
		expect(() => img.colorAt(0, height)).toThrowError(/are outside the bitmap/);
		expect(() => img.colorAt(0, height-1)).not.toThrow();
		expect(() => img.colorAt(width, 0)).toThrowError(/are outside the bitmap/);
		expect(() => img.colorAt(9999999999999, 0)).toThrowError(/are outside the bitmap/);
		expect(() => img.colorAt(0, 9999999999999)).toThrowError(/are outside the bitmap/);
	});

	it('Reads padded rows without crossing into row padding.', function() {
		const img = makeBitmap([
			['ff0000', '00ff00', '0000ff'],
			['ffff00', 'ff00ff', '00ffff']
		], { byteWidth: 16, fill: 0xEE });

		expect(img.colorAt(0, 0)).toEqual('ff0000');
		expect(img.colorAt(1, 0)).toEqual('00ff00');
		expect(img.colorAt(2, 0)).toEqual('0000ff');
		expect(img.colorAt(0, 1)).toEqual('ffff00');
		expect(img.colorAt(1, 1)).toEqual('ff00ff');
		expect(img.colorAt(2, 1)).toEqual('00ffff');
	});

	it('Finds colors inside sub-rectangles with non-zero origins.', function() {
		const img = makeBitmap([
			['111111', '222222', '333333', '444444'],
			['555555', '666666', '777777', 'ff00ff'],
			['888888', 'ff00ff', '999999', 'aaaaaa']
		], { byteWidth: 20, fill: 0xEE });

		expect(img.findColor('ff00ff')).toEqual({ x: 3, y: 1 });
		expect(img.findColor('ff00ff', { x: 1, y: 2, width: 1, height: 1 })).toEqual({ x: 1, y: 2 });
		expect(img.findColors('ff00ff', { x: 1, y: 1, width: 3, height: 2 })).toEqual([
			{ x: 3, y: 1 },
			{ x: 1, y: 2 }
		]);
		expect(img.countColor('ff00ff', { x: 1, y: 1, width: 3, height: 2 })).toEqual(2);
		expect(img.findColor('abcdef')).toBeNull();
	});

	it('Finds bitmaps inside padded haystacks and respects search rect origins.', function() {
		const haystack = makeBitmap([
			['101010', '101010', '101010', '101010', '101010', '101010'],
			['101010', 'ff0000', '0000ff', 'ff0000', '0000ff', '101010'],
			['101010', '00ff00', 'ffff00', '00ff00', 'ffff00', '101010'],
			['101010', '101010', '101010', '101010', '101010', '101010']
		], { byteWidth: 28, fill: 0xEE });
		const needle = makeBitmap([
			['ff0000', '0000ff'],
			['00ff00', 'ffff00']
		], { byteWidth: 12, fill: 0xEE });

		expect(haystack.findImage(needle)).toEqual({ x: 1, y: 1 });
		expect(haystack.findImages(needle, { x: 1, y: 1, width: 4, height: 2 })).toEqual([
			{ x: 1, y: 1 },
			{ x: 3, y: 1 }
		]);
		expect(haystack.countImage(needle, { x: 1, y: 1, width: 4, height: 2 })).toEqual(2);
		expect(haystack.findImage(needle, { x: 0, y: 0, width: 1, height: 4 })).toBeNull();
	});
});
