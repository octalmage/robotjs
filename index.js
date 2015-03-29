var robotjs = require('./build/Release/robotjs.node');

module.exports = robotjs;

module.exports.screen = {};

function bitmap (imagedata, width, height) 
{
    this.imagedata = imagedata;
    this.width = width;
    this.height = height;
}

bitmap.prototype.save = function(path) 
{
    console.log("saving");
    //robotjs.saveBitmap(this.imagedata, path);
};

module.exports.screen.capture = function()
{
    b = robotjs.captureScreen();
    return new bitmap(b.image, b.width, b.height);
}