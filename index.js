const robotjs = require('./build/Release/robotjs.node');

module.exports = robotjs;

module.exports.screen = {};
module.exports.image = {};

function Bitmap(width, height, byteWidth, bitsPerPixel, bytesPerPixel, image)
{   
    // Convenience constructor for when an object is passed (return of native functions).
    if (typeof width === 'object' && width !== null) {
        this.screenX = width.screenX;
        this.screenY = width.screenY;
        this.scaleX = width.scaleX;
        this.scaleY = width.scaleY;
        image = width.image;
        bytesPerPixel = width.bytesPerPixel;
        bitsPerPixel = width.bitsPerPixel;
        byteWidth = width.byteWidth;
        height = width.height;
        width = width.width;
    }

    this.width = width;
    this.height = height;
    this.byteWidth = byteWidth;
    this.bitsPerPixel = bitsPerPixel;
    this.bytesPerPixel = bytesPerPixel;
    this.image = image;
}

function Image(width, height, byteWidth, bitsPerPixel, bytesPerPixel, image)
{
    Bitmap.call(this, width, height, byteWidth, bitsPerPixel, bytesPerPixel, image);
}

Image.prototype = Bitmap.prototype;
Image.prototype.constructor = Image;

function getTargetDimensions(target)
{
    if (!target || typeof target !== 'object') {
        return { width: 0, height: 0 };
    }

    if (typeof target.width === 'number' && typeof target.height === 'number') {
        return { width: target.width, height: target.height };
    }

    return { width: 0, height: 0 };
}

Image.prototype.colorAt = function(x, y)
{
    return robotjs.getColor(this, x, y);
};

Image.prototype.findColor = function(color, options)
{
    return robotjs.findColor(this, color, options);
};

Image.prototype.findColors = function(color, options)
{
    return robotjs.findColors(this, color, options);
};

Image.prototype.countColor = function(color, options)
{
    return robotjs.countColor(this, color, options);
};

Image.prototype.findImage = function(needle, options)
{
    return robotjs.findImage(this, needle, options);
};

Image.prototype.findImages = function(needle, options)
{
    return robotjs.findImages(this, needle, options);
};

Image.prototype.countImage = function(needle, options)
{
    return robotjs.countImage(this, needle, options);
};

Image.prototype.save = function(path)
{
    return robotjs.saveImage(this, path);
};

Image.prototype.toScreenPoint = function(point, target)
{
    var dimensions = getTargetDimensions(target);
    var scaleX = typeof this.scaleX === 'number' && this.scaleX > 0 ? this.scaleX : 1;
    var scaleY = typeof this.scaleY === 'number' && this.scaleY > 0 ? this.scaleY : 1;
    var screenX = typeof this.screenX === 'number' ? this.screenX : 0;
    var screenY = typeof this.screenY === 'number' ? this.screenY : 0;

    return {
        x: Math.round(screenX + ((point.x + Math.floor(dimensions.width / 2)) / scaleX)),
        y: Math.round(screenY + ((point.y + Math.floor(dimensions.height / 2)) / scaleY))
    };
};

Image.prototype.click = function(point, target, button, double)
{
    var screenPoint = this.toScreenPoint(point, target);

    module.exports.moveMouse(screenPoint.x, screenPoint.y);
    if (typeof button === 'undefined') {
        module.exports.mouseClick();
    } else if (typeof double === 'undefined') {
        module.exports.mouseClick(button);
    } else {
        module.exports.mouseClick(button, double);
    }

    return screenPoint;
};

Image.prototype.clickImage = function(target, options, button, double)
{
    var match = this.findImage(target, options);

    if (!match) {
        return null;
    }

    this.click(match, target, button, double);
    return match;
};

module.exports.Image = Image;
module.exports.Bitmap = Image;

module.exports.screen.capture = function(x, y, width, height)
{
    //If coords have been passed, use them.
    if (typeof x !== "undefined" && typeof y !== "undefined" && typeof width !== "undefined" && typeof height !== "undefined")
    {
        return new Image(robotjs.captureScreen(x, y, width, height));
    }

    return new Image(robotjs.captureScreen());
};

module.exports.image.load = function(path)
{
    return new Image(robotjs.loadImage(path));
};

module.exports.image.save = function(bitmap, path)
{
    return robotjs.saveImage(bitmap, path);
};

module.exports.image.supportsPNG = !!robotjs.hasPNGSupport;
