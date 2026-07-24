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

	it('Finds overlapping images through the final valid row and column.', function() {
		const haystack = makeBitmap([
			['aabbcc', 'aabbcc', 'aabbcc', '101010'],
			['101010', '101010', 'aabbcc', 'aabbcc']
		]);
		const needle = makeBitmap([
			['aabbcc', 'aabbcc']
		]);

		expect(haystack.findImages(needle)).toEqual([
			{ x: 0, y: 0 },
			{ x: 1, y: 0 },
			{ x: 2, y: 1 }
		]);
		expect(haystack.countImage(needle)).toEqual(3);
		expect(haystack.findImage(needle, { x: 1, y: 0 })).toEqual({ x: 1, y: 0 });
	});

	it('Rejects colors and images beyond the configured tolerance.', function() {
		const first = makeBitmap([['101010']]);
		const second = makeBitmap([['151010']]);

		expect(first.findColor('151010', { tolerance: 0.01 })).toBeNull();
		expect(first.findImage(second, { tolerance: 0.01 })).toBeNull();
		expect(first.findColor('151010', { tolerance: 0.02 })).toEqual({ x: 0, y: 0 });
		expect(first.findImage(second, { tolerance: 0.02 })).toEqual({ x: 0, y: 0 });
	});

	it('Accepts hash-prefixed colors and rejects malformed color values.', function() {
		const bitmap = makeBitmap([['123456']]);

		expect(bitmap.findColor('#123456')).toEqual({ x: 0, y: 0 });
		['12345', '1234567', 'gggggg', 123456].forEach(function(color) {
			expect(function() {
				bitmap.findColor(color);
			}).toThrowError(/Color must be a 6-digit hex string/);
		});
	});

	it('Applies non-zero color tolerance symmetrically for every channel.', function() {
		const tolerance = 0.01;
		const channelPairs = [
			['101010', '111010'],
			['101010', '101110'],
			['101010', '101011']
		];

		channelPairs.forEach(function(pair) {
			expect(makeBitmap([[pair[0]]]).findColor(pair[1], { tolerance: tolerance }))
				.toEqual({ x: 0, y: 0 });
			expect(makeBitmap([[pair[1]]]).findColor(pair[0], { tolerance: tolerance }))
				.toEqual({ x: 0, y: 0 });
		});
	});

	it('Applies non-zero image tolerance symmetrically across mixed channel deltas.', function() {
		const first = makeBitmap([
			['101010', '202020'],
			['303030', '404040']
		]);
		const second = makeBitmap([
			['111010', '201f20'],
			['303031', '3f4040']
		]);
		const options = { tolerance: 0.01 };

		expect(first.findImage(second, options)).toEqual({ x: 0, y: 0 });
		expect(second.findImage(first, options)).toEqual({ x: 0, y: 0 });
	});

	[
		{ name: 'NaN', value: NaN, error: /must be a finite integer/ },
		{ name: 'Infinity', value: Infinity, error: /must be a finite integer/ },
		{ name: 'fractional numbers', value: 0.5, error: /must be a finite integer/ },
		{ name: 'numbers outside the native range', value: Number.MAX_VALUE, error: /outside the supported range/ }
	].forEach(function(valueCase) {
		it('Rejects ' + valueCase.name + ' in every search rectangle field.', function() {
			const bitmap = makeBitmap([
				['123456', 'abcdef'],
				['fedcba', '654321']
			]);
			const needle = makeBitmap([['123456']]);

			['x', 'y', 'width', 'height'].forEach(function(field) {
				const options = {};
				options[field] = valueCase.value;

				expect(function() {
					bitmap.findColor('123456', options);
				}).toThrowError(valueCase.error);
				expect(function() {
					bitmap.findImage(needle, options);
				}).toThrowError(valueCase.error);
			});
		});
	});

	it('Rejects non-finite and out-of-range search tolerances.', function() {
		const bitmap = makeBitmap([['123456']]);
		const needle = makeBitmap([['123456']]);
		const invalidTolerances = [NaN, Infinity, Number.MAX_VALUE, -0.01, 1.01];

		invalidTolerances.forEach(function(tolerance) {
			const options = { tolerance: tolerance };

			expect(function() {
				bitmap.findColor('123456', options);
			}).toThrowError(/tolerance must be between 0\.0 and 1\.0/);
			expect(function() {
				bitmap.findImage(needle, options);
			}).toThrowError(/tolerance must be between 0\.0 and 1\.0/);
		});
	});

	[
		{
			name: 'NaN dimensions',
			change: function(bitmap) { bitmap.width = NaN; },
			error: /width must be a finite integer/
		},
		{
			name: 'infinite dimensions',
			change: function(bitmap) { bitmap.height = Infinity; },
			error: /height must be a finite integer/
		},
		{
			name: 'fractional row strides',
			change: function(bitmap) { bitmap.byteWidth = 3.5; },
			error: /byteWidth must be a finite integer/
		},
		{
			name: 'dimensions outside the native range',
			change: function(bitmap) { bitmap.width = Number.MAX_VALUE; },
			error: /width is outside the supported range/
		},
		{
			name: 'bitsPerPixel values that wrap through uint8',
			change: function(bitmap) { bitmap.bitsPerPixel = 288; },
			error: /pixel metadata is outside the supported range/
		},
		{
			name: 'bytesPerPixel values that wrap through uint8',
			change: function(bitmap) {
				bitmap.bitsPerPixel = 24;
				bitmap.bytesPerPixel = 259;
			},
			error: /pixel metadata is outside the supported range/
		},
		{
			name: 'inconsistent pixel metadata',
			change: function(bitmap) { bitmap.bitsPerPixel = 24; },
			error: /must use 24-bit or 32-bit pixels/
		},
		{
			name: 'row strides smaller than their pixels',
			change: function(bitmap) { bitmap.byteWidth = 3; },
			error: /byteWidth is smaller than the bitmap row size/
		},
		{
			name: 'buffers smaller than their declared metadata',
			change: function(bitmap) { bitmap.image = Buffer.alloc(3); },
			error: /image buffer is smaller than byteWidth \* height/
		}
	].forEach(function(testCase) {
		it('Rejects malformed bitmap metadata with ' + testCase.name + '.', function() {
			const bitmap = makeBitmap([['123456']]);
			testCase.change(bitmap);

			expect(function() {
				bitmap.colorAt(0, 0);
			}).toThrowError(testCase.error);
		});
	});

	it('Returns the documented negative result for every search variant.', function() {
		const bitmap = makeBitmap([
			['101010', '202020'],
			['303030', '404040']
		]);
		const needle = makeBitmap([['ffffff']]);

		expect(bitmap.findColor('ffffff')).toBeNull();
		expect(bitmap.findColors('ffffff')).toEqual([]);
		expect(bitmap.countColor('ffffff')).toEqual(0);
		expect(bitmap.findImage(needle)).toBeNull();
		expect(bitmap.findImages(needle)).toEqual([]);
		expect(bitmap.countImage(needle)).toEqual(0);
	});
});
