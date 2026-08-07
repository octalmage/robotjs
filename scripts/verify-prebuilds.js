'use strict';

const fs = require('fs');
const path = require('path');

const expected = [
    'darwin-arm64/node.napi.node',
    'darwin-x64/node.napi.node',
    'linux-arm64/node.napi.glibc.node',
    'linux-x64/node.napi.glibc.node',
    'win32-arm64/node.napi.node',
    'win32-x64/node.napi.node'
];

const missing = expected.filter((relativePath) => {
    const absolutePath = path.join(__dirname, '..', 'prebuilds', relativePath);

    try {
        const stats = fs.statSync(absolutePath);
        return !stats.isFile() || stats.size === 0;
    } catch (error) {
        if (error.code === 'ENOENT') {
            return true;
        }
        throw error;
    }
});

if (missing.length > 0) {
    throw new Error(`Missing required prebuilds:\n${missing.join('\n')}`);
}

console.log(`Verified ${expected.length} required prebuilds.`);
