const fs = require('fs');
const os = require('os');
const path = require('path');
const robot = require('..');
const { create24BitBMP, makeBitmap } = require('./helpers/images');

describe('Image', function() {
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

	it('Rejects unsupported image extensions.', function() {
		expect(function() {
			robot.image.load('/tmp/not-an-image.txt');
		}).toThrowError(/Unsupported image type/);
	});

	it('Saves and reloads BMP bitmaps.', function() {
		const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'robotjs-image-save-'));
		const bmpPath = path.join(tmpDir, 'saved.bmp');
		const bitmap = makeBitmap([
			['112233', '445566'],
			['778899', 'aabbcc']
		], { byteWidth: 12, bytesPerPixel: 3 });
		let bmpReloaded;

		try {
			expect(robot.image.save(bitmap, bmpPath)).toBe(true);
			expect(fs.existsSync(bmpPath)).toBe(true);

			bmpReloaded = robot.image.load(bmpPath);

			expect(bmpReloaded.colorAt(0, 0)).toEqual('112233');
			expect(bmpReloaded.colorAt(1, 1)).toEqual('aabbcc');
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

	it('Rejects unsupported save extensions.', function() {
		const bitmap = makeBitmap([
			['123456']
		]);

		expect(function() {
			robot.image.save(bitmap, '/tmp/not-an-image.txt');
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

	it('Clicks a found image using screen-space coordinates.', function() {
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
			match = capture.clickImage(needle, { tolerance: 0 }, 'left', true);
			expect(match).toEqual({ x: 1, y: 1 });
			expect(movedTo).toEqual({ x: 11, y: 21 });
			expect(clickedWith).toEqual({ button: 'left', double: true });
		} finally {
			robot.moveMouse = originalMoveMouse;
			robot.mouseClick = originalMouseClick;
		}
	});
});
