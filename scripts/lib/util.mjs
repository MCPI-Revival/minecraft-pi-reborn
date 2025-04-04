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
export function doesPackageExist(name) {
    try {
        info('Checking If Package Exists: ' + name);
        child_process.execFileSync('apt-cache', ['show', name], {stdio: 'ignore'});
        return true;
    } catch (e) {
        return false;
    }
}

// Create Directory
export function createDir(dir, clean) {
    if (clean) {
        fs.rmSync(dir, {recursive: true, force: true});
    }
    fs.mkdirSync(dir, {recursive: true});
}

// Get System Information
export function getDebianVersion() {
    const info = fs.readFileSync('/etc/os-release', 'utf8');
    const lines = info.split('\n');
    const prefix = 'VERSION_CODENAME=';
    for (const line of lines) {
        if (line.startsWith(prefix)) {
            return line.substring(prefix.length);
        }
    }
    return 'unknown';
}

// Get Scripts Directory
export function getScriptsDir() {
    const __filename = url.fileURLToPath(import.meta.url);
    const __dirname = path.dirname(__filename);
    return path.join(__dirname, '..');
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