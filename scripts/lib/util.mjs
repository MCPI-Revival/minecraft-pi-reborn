import * as child_process from 'node:child_process';
import * as fs from 'node:fs';
import * as path from 'node:path';
import * as url from 'node:url';

// Logging
const EXIT_FAILURE = 1;
export function fail(message) {
    console.error(message);
    process.exit(EXIT_FAILURE);
}
export function err(message) {
    fail('ERROR: ' + message);
}
export function info(message) {
    console.log('INFO: ' + message);
}

// Sub-Process
export function run(command) {
    try {
        info('Running: ' + command.join(' '));
        child_process.execFileSync(command[0], command.slice(1), {stdio: 'inherit'});
    } catch (e) {
        err(e);
    }
}

// Create Directory
export function createDir(dir, clean) {
    if (clean) {
        fs.rmSync(dir, {recursive: true, force: true});
    }
    fs.mkdirSync(dir, {recursive: true});
}

// Get Scripts Directory
export function getScriptsDir() {
    const __filename = url.fileURLToPath(import.meta.url);
    const __dirname = path.dirname(__filename);
    return path.join(__dirname, '..');
}
export function readFile(...args) {
    const __dirname = getScriptsDir();
    const root = path.join(__dirname, '..');
    return {
        data: fs.readFileSync(path.join(root, ...args), 'utf8').trim(),
        time: fs.statSync(path.join(root, ...args)).mtime
    };
}
export function getVersion() {
    return readFile('VERSION');
}

// Custom Build Tools
export function getBuildToolsDir() {
    return path.join(getScriptsDir(), '..', 'tools');
}
export function getBuildToolsBin() {
    const dir = path.join(getBuildToolsDir(), 'bin');
    createDir(dir, false);
    return dir;
}