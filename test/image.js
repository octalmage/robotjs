const fs = require('fs');
const os = require('os');
const path = require('path');
const robot = require('..');
const { create24BitBMP, create8BitPNG, makeBitmap } = require('./helpers/images');

describe('Image', function() {
	const palette = [
		0xFF, 0x00, 0x00,
		0x00, 0xFF, 0x00,
		0x00, 0x00, 0xFF,
		0xFF, 0xFF, 0x00
	];
	const colorRows = [
		['ff0000', '00ff00'],
		['0000ff', 'ffff00']
	];
	const pngCases = [
		{
			name: 'RGB',
			colorType: 2,
			rows: [
				[0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00],
				[0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00]
			],
			expected: colorRows
		},
		{
			name: 'RGBA',
			colorType: 6,
			rows: [
				[0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x55],
				[0x00, 0x00, 0xFF, 0xAA, 0xFF, 0xFF, 0x00, 0xFF]
			],
			expected: colorRows
		},
		{
			name: 'grayscale',
			colorType: 0,
			rows: [
				[0x10, 0x80],
				[0xE0, 0x40]
			],
			expected: [
				['101010', '808080'],
				['e0e0e0', '404040']
			]
		},
		{
			name: 'grayscale-alpha',
			colorType: 4,
			rows: [
				[0x10, 0x00, 0x80, 0x55],
				[0xE0, 0xAA, 0x40, 0xFF]
			],
			expected: [
				['101010', '808080'],
				['e0e0e0', '404040']
			]
		},
		{
			name: 'palette',
			colorType: 3,
			palette: palette,
			rows: [
				[0, 1],
				[2, 3]
			],
			expected: colorRows
		},
		{
			name: 'palette with tRNS',
			colorType: 3,
			palette: palette,
			transparency: [0x00, 0x55, 0xAA, 0xFF],
			rows: [
				[0, 1],
				[2, 3]
			],
			expected: colorRows
		}
	];
	it('Loads a BMP file and uses it as a search bitmap.', function() {
		const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'robotjs-image-'));
		const needlePath = path.join(tmpDir, 'needle.bmp');
		const needleRows = [
			['ff0000', '0000ff'],
			['00ff00', 'ffff00']
		];
		const haystack = makeBitmap([
			['111111', '111111', '111111', '111111'],
			['111111', 'ff0000', '0000ff', '111111'],
			['111111', '00ff00', 'ffff00', '111111']
		], { byteWidth: 20 });
		let needle;

		fs.writeFileSync(needlePath, create24BitBMP(needleRows));

		try {
			needle = robot.image.load(needlePath);
			expect(needle.width).toEqual(2);
			expect(needle.height).toEqual(2);
			expect(needle.colorAt(0, 0)).toEqual('ff0000');
			expect(needle.colorAt(1, 1)).toEqual('ffff00');
			expect(haystack.findImage(needle)).toEqual({ x: 1, y: 1 });
		} finally {
			fs.rmSync(tmpDir, { recursive: true, force: true });
		}
	});

	it('Rejects unsupported four-character load extensions.', function() {
		expect(function() {
			robot.image.load('/tmp/not-an-image.jpeg');
		}).toThrowError(/Unsupported image type/);
	});

	it('Saves BMP files with consistent headers and pixels.', function() {
		const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'robotjs-image-save-'));
		const bmpPath = path.join(tmpDir, 'saved.bmp');
		const rows = [
			['112233', '445566', '778899'],
			['aabbcc', 'ddeeff', '123456']
		];
		const bitmap = makeBitmap(rows, { byteWidth: 12, bytesPerPixel: 3 });
		let bmpReloaded;

		try {
			expect(robot.image.save(bitmap, bmpPath)).toBe(true);
			expect(fs.existsSync(bmpPath)).toBe(true);

			const buffer = fs.readFileSync(bmpPath);
			// BMP header layout: https://learn.microsoft.com/en-us/windows/win32/gdi/bitmap-storage
			const declaredFileSize = buffer.readUInt32LE(2);
			const pixelOffset = buffer.readUInt32LE(10);
			const dibHeaderSize = buffer.readUInt32LE(14);
			const width = buffer.readInt32LE(18);
			const height = Math.abs(buffer.readInt32LE(22));
			const bytesPerPixel = buffer.readUInt16LE(28) / 8;
			const declaredImageSize = buffer.readUInt32LE(34);
			const rowStride = ((width * bytesPerPixel) + 3) & ~3;

			expect(buffer.length).toEqual(declaredFileSize);
			expect(pixelOffset).toEqual(14 + dibHeaderSize);
			expect(pixelOffset).toEqual(54);
			expect(rowStride).toEqual(12);
			expect(rowStride).toBeGreaterThan(width * bytesPerPixel);
			expect(declaredImageSize).toEqual(rowStride * height);
			expect(declaredFileSize).toEqual(pixelOffset + declaredImageSize);

			bmpReloaded = robot.image.load(bmpPath);
			rows.forEach(function(row, y) {
				row.forEach(function(color, x) {
					expect(bmpReloaded.colorAt(x, y)).toEqual(color);
				});
			});
		} finally {
			fs.rmSync(tmpDir, { recursive: true, force: true });
		}
	});

	it('Saves and reloads PNG bitmaps when PNG support is enabled.', function() {
		let tmpDir, pngPath, bitmap, pngReloaded;

		if (!robot.image.supportsPNG) {
			pending('PNG support is not enabled in this build.');
		}

		tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'robotjs-image-png-'));
		pngPath = path.join(tmpDir, 'saved.png');
		bitmap = makeBitmap([
			['112233', '445566'],
			['778899', 'aabbcc']
		], { byteWidth: 12, bytesPerPixel: 3 });

		try {
			expect(bitmap.save(pngPath)).toBe(true);
			expect(fs.existsSync(pngPath)).toBe(true);

			pngReloaded = robot.image.load(pngPath);
			expect(pngReloaded.colorAt(0, 1)).toEqual('778899');
			expect(pngReloaded.colorAt(1, 0)).toEqual('445566');
		} finally {
			fs.rmSync(tmpDir, { recursive: true, force: true });
		}
	});

	pngCases.forEach(function(testCase) {
		it('Decodes a valid 2x2 ' + testCase.name + ' PNG.', function() {
			let tmpDir, pngPath, decoded;

			if (!robot.image.supportsPNG) {
				pending('PNG support is not enabled in this build.');
			}

			tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'robotjs-png-format-'));
			pngPath = path.join(tmpDir, testCase.name.replace(/[^a-z]+/gi, '-') + '.png');

			try {
				fs.writeFileSync(pngPath, create8BitPNG(testCase));
				decoded = robot.image.load(pngPath);

				expect([
					[decoded.colorAt(0, 0), decoded.colorAt(1, 0)],
					[decoded.colorAt(0, 1), decoded.colorAt(1, 1)]
				]).toEqual(testCase.expected);
			} finally {
				fs.rmSync(tmpDir, { recursive: true, force: true });
			}
		});
	});

	it('Rejects PNG operations when PNG support is disabled.', function() {
		const bitmap = makeBitmap([
			['123456']
		]);

		if (robot.image.supportsPNG) {
			pending('PNG support is enabled in this build.');
		}

		expect(function() {
			robot.image.load('/tmp/example.png');
		}).toThrowError(/PNG support is not enabled/);

		expect(function() {
			bitmap.save('/tmp/example.png');
		}).toThrowError(/PNG support is not enabled/);
	});

	it('Rejects unsupported four-character save extensions.', function() {
		const bitmap = makeBitmap([
			['123456']
		]);

		expect(function() {
			robot.image.save(bitmap, '/tmp/not-an-image.jpeg');
		}).toThrowError(/Unsupported image type/);
	});

	it('Converts capture-space points into screen-space points.', function() {
		const capture = makeBitmap([
			['123456', 'abcdef']
		]);
		const needle = makeBitmap([
			['ffffff', '000000'],
			['000000', 'ffffff']
		]);

		capture.screenX = 100;
		capture.screenY = 200;
		capture.scaleX = 2;
		capture.scaleY = 2;

		expect(capture.toScreenPoint({ x: 20, y: 10 })).toEqual({ x: 110, y: 205 });
		expect(capture.toScreenPoint({ x: 20, y: 10 }, needle)).toEqual({ x: 111, y: 206 });
	});

	it('Converts target centers from captures with negative origins and non-uniform scaling.', function() {
		const capture = makeBitmap([
			['123456']
		]);
		const targetDimensions = {
			width: 48,
			height: 40
		};

		capture.screenX = -1440;
		capture.screenY = -900;
		capture.scaleX = 1.5;
		capture.scaleY = 2;

		expect(capture.toScreenPoint({ x: 300, y: 200 }, targetDimensions)).toEqual({
			x: -1224,
			y: -790
		});
	});

	it('findColors iterates across row boundaries correctly.', function() {
		// Regression: ITER_NEXT_POINT previously hardcoded the letiable name
		// "point" instead of using its macro parameter. This test places
		// matching pixels on separate rows so the macro must increment y
		// when x wraps past the row end.
		const bitmap = makeBitmap([
			['ff0000', '111111', '111111'],
			['111111', '111111', 'ff0000'],
			['111111', 'ff0000', '111111']
		]);

		const results = bitmap.findColors('ff0000');
		expect(results.length).toEqual(3);
		expect(results).toEqual([
			{ x: 0, y: 0 },
			{ x: 2, y: 1 },
			{ x: 1, y: 2 }
		]);
	});

	it('findImages iterates across row boundaries correctly.', function() {
		// Same regression coverage for the bitmap search path.
		const needle = makeBitmap([
			['aabbcc']
		]);
		const haystack = makeBitmap([
			['aabbcc', '111111', '111111'],
			['111111', '111111', 'aabbcc'],
			['111111', 'aabbcc', '111111']
		]);

		const results = haystack.findImages(needle);
		expect(results.length).toEqual(3);
		expect(results).toEqual([
			{ x: 0, y: 0 },
			{ x: 2, y: 1 },
			{ x: 1, y: 2 }
		]);
	});

	it('Does not move or click when clickImage cannot find its target.', function() {
		const capture = makeBitmap([['123456']]);
		const needle = makeBitmap([['abcdef']]);
		const originalMoveMouse = robot.moveMouse;
		const originalMouseClick = robot.mouseClick;
		let moved = false;
		let clicked = false;

		robot.moveMouse = function() {
			moved = true;
		};
		robot.mouseClick = function() {
			clicked = true;
		};

		try {
			expect(capture.clickImage(needle)).toBeNull();
			expect(moved).toBe(false);
			expect(clicked).toBe(false);
		} finally {
			robot.moveMouse = originalMoveMouse;
			robot.mouseClick = originalMouseClick;
		}
	});

	it('Defaults an omitted button to left for direct double clicks.', function() {
		const capture = makeBitmap([['123456']]);
		const originalMoveMouse = robot.moveMouse;
		const originalMouseClick = robot.mouseClick;
		let clickedWith;

		robot.moveMouse = function() {};
		robot.mouseClick = function(button, double) {
			clickedWith = { button: button, double: double };
		};

		try {
			capture.click({ x: 0, y: 0 }, undefined, undefined, true);
			expect(clickedWith).toEqual({ button: 'left', double: true });
		} finally {
			robot.moveMouse = originalMoveMouse;
			robot.mouseClick = originalMouseClick;
		}
	});

	it('Defaults an omitted button to left when double-clicking a found image.', function() {
		const capture = makeBitmap([
			['111111', '111111', '111111', '111111'],
			['111111', 'ff0000', '0000ff', '111111'],
			['111111', '00ff00', 'ffff00', '111111']
		], { byteWidth: 20 });
		const needle = makeBitmap([
			['ff0000', '0000ff'],
			['00ff00', 'ffff00']
		], { byteWidth: 12 });
		const originalMoveMouse = robot.moveMouse;
		const originalMouseClick = robot.mouseClick;
		let movedTo;
		let clickedWith;
		let match;

		capture.screenX = 10;
		capture.screenY = 20;
		capture.scaleX = 2;
		capture.scaleY = 2;

		robot.moveMouse = function(x, y) {
			movedTo = { x: x, y: y };
		};
		robot.mouseClick = function(button, double) {
			clickedWith = { button: button, double: double };
		};

		try {
			match = capture.clickImage(needle, { tolerance: 0 }, undefined, true);
			expect(match).toEqual({ x: 1, y: 1 });
			expect(movedTo).toEqual({ x: 11, y: 21 });
			expect(clickedWith).toEqual({ button: 'left', double: true });
		} finally {
			robot.moveMouse = originalMoveMouse;
			robot.mouseClick = originalMouseClick;
		}
	});
});
