var robot = require('..');

try {
    robot.keyTap('a', [])
    robot.keyTap('a')
    robot.keyTap('a', ['shift'])
} catch (e) {
    console.log(e)
}