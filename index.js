var robotjs = require('./build/Release/robotjs.node');

module.exports = robotjs;

module.exports.screen = {};

function bitmap(width, height, byteWidth, bitsPerPixel, bytesPerPixel, image) 
{
    this.width = width;
    this.height = height;
    this.byteWidth = byteWidth;
    this.bitsPerPixel = bitsPerPixel;
    this.bytesPerPixel = bytesPerPixel;
    this.image = image;
    
    this.colorAt = function(x, y)
    {
        return robotjs.getColor(this, x, y);
    };
}

module.exports.screen.capture = function(x, y, width, height)
{
    //If coords have been passed, use them.
    if (typeof x !== "undefined" && typeof y !== "undefined" && typeof width !== "undefined" && typeof height !== "undefined")
    {
        b = robotjs.captureScreen(x, y, width, height);
    }
    else 
    {
        b = robotjs.captureScreen();
    }
    
    return new bitmap(b.width, b.height, b.byteWidth, b.bitsPerPixel, b.bytesPerPixel, b.image);
};