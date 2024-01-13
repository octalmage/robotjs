const path = require('path');
const os = require('os');
const { rebuild } = require('@electron/rebuild');
const yargs = require('yargs');

const argv = yargs(process.argv)
  .options({
    electron: { type: 'string', demand: true },
    forceabi: { type: 'number', demand: true },
    arch: {
      alias: 'a',
      describe: 'The architecture you want to build for',
      choices: ['x64', 'arm64'],
    }
  })
  .parseSync();

async function run() {
  console.log('run()');

  try {
    await rebuild({
      buildPath: path.join(__dirname, '..'),
      electronVersion: argv.electron,
      onlyModules: ['robotjs'],
      forceABI: argv.forceabi,
      force: true,
      arch: argv.arch ?? os.arch(),
    });

    console.log('run() | succeeded');
  } catch (error) {
    console.warn(`run() | failed: ${error}`);

    throw error;
  }
}

run();
