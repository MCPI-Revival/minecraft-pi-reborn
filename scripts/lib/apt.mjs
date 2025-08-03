import * as child_process from 'node:child_process';
import * as fs from 'node:fs';
import { err, info, run } from './util.mjs';
import { Architectures } from './options.mjs';

// Check System
if (process.getuid() !== 0) {
    err("Must Run As Root!");
}
if (!fs.existsSync('/etc/debian_version')) {
    err('Non-Debian OS Detected');
}

// Check If Package Exists
export function doesPackageExist(name) {
    try {
        info('Checking If Package Exists: ' + name);
        child_process.execFileSync('apt-cache', ['show', name], {stdio: 'ignore'});
        return true;
    } catch (e) {
        return false;
    }
}

// Update APT
let archSuffix = '';
export function setupApt(architecture) {
    if (architecture !== Architectures.Host) {
        const arch = architecture.name;
        run(['dpkg', '--add-architecture', arch]);
        archSuffix = ':' + arch;
    }
    run(['apt-get', 'update']);
    run(['apt-get', 'dist-upgrade', '-y']);
}

// Install Packages
const packages = [];
export function addPackageForBuild(...arr) {
    // This will install packages that match
    // the build machine's architecture.
    // This is usually used for build tools.
    packages.push(...arr);
}
export function getPackageForHost(name) {
    return name + archSuffix;
}
export function addPackageForHost(...arr) {
    // This will install packages that match
    // the host machine's architecture.
    // This is usually used for libraries.
    for (const name of arr) {
        packages.push(getPackageForHost(name));
    }
}
export function installPackages() {
    // Install Queued Packages
    info('Installing ' + packages.length + ' Packages...');
    run(['apt-get', 'install', '--no-install-recommends', '-y', ...packages]);
}