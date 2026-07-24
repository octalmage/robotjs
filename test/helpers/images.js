const robot = require('../..');
const zlib = require('zlib');

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

function crc32(buffer) {
	let checksum = 0xFFFFFFFF;

	for (let index = 0; index < buffer.length; index += 1) {
		checksum ^= buffer[index];
		for (let bit = 0; bit < 8; bit += 1) {
			checksum = (checksum >>> 1) ^ ((checksum & 1) ? 0xEDB88320 : 0);
		}
	}

	return (checksum ^ 0xFFFFFFFF) >>> 0;
}

function createPNGChunk(type, data) {
	const typeBytes = Buffer.from(type, 'ascii');
	const checksumInput = Buffer.concat([typeBytes, data]);
	const chunk = Buffer.alloc(data.length + 12);

	chunk.writeUInt32BE(data.length, 0);
	typeBytes.copy(chunk, 4);
	data.copy(chunk, 8);
	chunk.writeUInt32BE(crc32(checksumInput), data.length + 8);
	return chunk;
}

function create8BitPNG(options) {
	const header = Buffer.alloc(13, 0x00);
	const scanlines = options.rows.map(function(row) {
		return Buffer.concat([Buffer.from([0]), Buffer.from(row)]);
	});
	const chunks = [];

	header.writeUInt32BE(2, 0);
	header.writeUInt32BE(2, 4);
	header[8] = 8;
	header[9] = options.colorType;
	chunks.push(createPNGChunk('IHDR', header));

	if (options.palette) {
		chunks.push(createPNGChunk('PLTE', Buffer.from(options.palette)));
	}
	if (options.transparency) {
		chunks.push(createPNGChunk('tRNS', Buffer.from(options.transparency)));
	}

	chunks.push(createPNGChunk('IDAT', zlib.deflateSync(Buffer.concat(scanlines))));
	chunks.push(createPNGChunk('IEND', Buffer.alloc(0)));

	return Buffer.concat([
		Buffer.from([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A])
	].concat(chunks));
}

module.exports = {
	create24BitBMP: create24BitBMP,
	create8BitPNG: create8BitPNG,
	makeBitmap: makeBitmap
};
