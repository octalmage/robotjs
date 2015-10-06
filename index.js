var robotjs = require('./build/Release/robotjs.node');

module.exports = robotjs;

module.exports.screen = {};

function bitmap (width, height, byteWidth, bitsPerPixel, bytesPerPixel, image) 
{
    this.width = width;
    this.height = height;
	this.byteWidth = byteWidth;
	this.bitsPerPixel = bitsPerPixel;
	this.bytesPerPixel = bytesPerPixel;
	this.image = image;
}

bitmap.prototype.color = function()
{
	return robotjs.getColor(this);
};

module.exports.screen.capture = function()
{
    b = robotjs.captureScreen();
    return new bitmap(b.width, b.height, b.byteWidth, b.bitsPerPixel, b.bytesPerPixel, b.image);
};