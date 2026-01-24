import * as child_process from 'node:child_process';
import * as fs from 'node:fs';
import * as path from 'node:path';
import * as url from 'node:url';

// Logging
const EXIT_FAILURE = 1;
/**
 * @param {string} message
 */
export function fail(message) {
    console.error(message);
    process.exit(EXIT_FAILURE);
}
/**
 * @param {string} message
 */
export function err(message) {
    fail('ERROR: ' + message);
}
/**
 * @param {string} message
 */
export function info(message) {
    console.log('INFO: ' + message);
}

// Sub-Process
/**
 * @param {string[]} command
 */
export function run(command) {
    try {
        const prettyCommand = command
            .map(part => part.includes(' ') ? JSON.stringify(part) : part)
            .join(' ');
        info('Running: ' + prettyCommand);
        child_process.execFileSync(command[0], command.slice(1), {stdio: 'inherit'});
    } catch (e) {
        err(e);
    }
}

// Create Directory
/**
 * @param {string} dir
 * @param {boolean} clean
 */
export function createDir(dir, clean) {
    if (clean) {
        fs.rmSync(dir, {recursive: true, force: true});
    }
    fs.mkdirSync(dir, {recursive: true});
}

// Get Scripts Directory
/**
 * @returns {string}
 */
export function getScriptsDir() {
    const __filename = url.fileURLToPath(import.meta.url);
    const __dirname = path.dirname(__filename);
    return path.join(__dirname, '..');
}
/**
 * @typedef {object} FileData
 * @property {string} data
 * @property {Date} time
 */
/**
 * @param {string[]} args
 * @returns {FileData}
 */
export function readFile(...args) {
    const __dirname = getScriptsDir();
    const root = path.join(__dirname, '..');
    return {
        data: fs.readFileSync(path.join(root, ...args), 'utf8').trim(),
        time: fs.statSync(path.join(root, ...args)).mtime
    };
}
/**
 * @returns {FileData}
 */
export function getVersion() {
    return readFile('VERSION');
}

// Custom Build Tools
/**
 * @returns {string}
 */
export function getBuildToolsDir() {
    return path.join(getScriptsDir(), '..', 'tools');
}
/**
 * @returns {string}
 */
export function getBuildToolsBin() {
    const dir = path.join(getBuildToolsDir(), 'bin');
    createDir(dir, false);
    return dir;
}