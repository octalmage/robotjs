const robot = require('../..');

function writeColor(buffer, byteWidth, bytesPerPixel, x, y, hex) {
	const offset = (y * byteWidth) + (x * bytesPerPixel);
	const color = parseInt(hex, 16);

	buffer[offset] = color & 0xFF;
	buffer[offset + 1] = (color >> 8) & 0xFF;
	buffer[offset + 2] = (color >> 16) & 0xFF;
	if (bytesPerPixel === 4) {
		buffer[offset + 3] = 0xFF;
	}
}

function makeBitmap(rows, options) {
	options = options || {};

	const width = rows[0].length;
	const height = rows.length;
	const bytesPerPixel = options.bytesPerPixel || 4;
	const byteWidth = options.byteWidth || (width * bytesPerPixel);
	const fill = typeof options.fill === 'number' ? options.fill : 0x00;
	const image = Buffer.alloc(byteWidth * height, fill);

	rows.forEach(function(row, y) {
		row.forEach(function(hex, x) {
			writeColor(image, byteWidth, bytesPerPixel, x, y, hex);
		});
	});

	return new robot.Image(width, height, byteWidth, bytesPerPixel * 8, bytesPerPixel, image);
}

function create24BitBMP(rows) {
	const width = rows[0].length;
	const height = rows.length;
	const rowStride = (width * 3 + 3) & ~3;
	const imageSize = rowStride * height;
	const fileSize = 54 + imageSize;
	const buffer = Buffer.alloc(fileSize, 0x00);

	buffer.writeUInt16LE(0x4D42, 0);
	buffer.writeUInt32LE(fileSize, 2);
	buffer.writeUInt32LE(54, 10);

	buffer.writeUInt32LE(40, 14);
	buffer.writeInt32LE(width, 18);
	buffer.writeInt32LE(height, 22);
	buffer.writeUInt16LE(1, 26);
	buffer.writeUInt16LE(24, 28);
	buffer.writeUInt32LE(0, 30);
	buffer.writeUInt32LE(imageSize, 34);

	rows.slice().reverse().forEach(function(row, rowIndex) {
		const rowOffset = 54 + (rowIndex * rowStride);
		const rowBuffer = buffer.subarray(rowOffset, rowOffset + rowStride);

		row.forEach(function(hex, x) {
			writeColor(rowBuffer, rowStride, 3, x, 0, hex);
		});
	});

	return buffer;
}

module.exports = {
	create24BitBMP: create24BitBMP,
	makeBitmap: makeBitmap
};
