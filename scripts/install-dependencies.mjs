#!/usr/bin/env node
import * as path from 'node:path';
import * as fs from 'node:fs';
import { err, run, makeExecutable, getDebianVersion, getScriptsDir, info, doesPackageExist } from './lib/util.mjs';
import { parseOptions, Enum, Architectures } from './lib/options.mjs';

// Check System
if (process.getuid() !== 0) {
    err("Must Run As Root!");
}
if (!fs.existsSync('/etc/debian_version')) {
    err('Non-Debian OS Detected');
}

// Options
const Modes = new Enum([
    'Build',
    'Test',
    'SDK'
]);
const options = parseOptions([
    ['mode', Modes],
    ['architecture', Architectures]
], [], null);

// Setup Backports If Needed
const debianVersion = getDebianVersion();
info('OS Version: ' + debianVersion);
let backportsSuffix = '';
if (debianVersion === 'bullseye') {
    const repo = debianVersion + '-backports';
    const source = `deb http://deb.debian.org/debian ${repo} main\n`;
    fs.writeFileSync(`/etc/apt/sources.list.d/${repo}.list`, source);
    backportsSuffix = '/' + repo;
}

// Update APT
let archSuffix = '';
if (options.architecture !== Architectures.Host) {
    const arch = options.architecture.name;
    run(['dpkg', '--add-architecture', arch]);
    archSuffix = ':' + arch;
}
run(['apt-get', 'update']);
run(['apt-get', 'dist-upgrade', '-y']);

// Install Packages
const packages = [];
function addPackageForBuild(...arr) {
    // The machine the package is built on.
    packages.push(...arr);
}
function addPackageForHost(...arr) {
    // The machine the package is built for.
    for (const name of arr) {
        packages.push(name + archSuffix);
    }
}

// Build Dependencies
const handlers = new Map();
handlers.set(Modes.Build, function () {
    // Build Dependencies
    addPackageForBuild(
        'git',
        'cmake' + backportsSuffix,
        'ninja-build'
    );

    // Compiler
    if (options.architecture === Architectures.Host) {
        addPackageForBuild('gcc', 'g++');
    } else {
        addPackageForBuild('crossbuild-essential-' + options.architecture.name);
    }

    // Main Dependencies
    addPackageForHost('libopenal-dev');

    // GLFW Dependencies
    addPackageForBuild('libwayland-bin');
    addPackageForHost(
        'libwayland-dev',
        'libxkbcommon-dev',
        'libx11-dev',
        'libxcursor-dev',
        'libxi-dev',
        'libxinerama-dev',
        'libxrandr-dev',
        'libxext-dev'
    );

    // QEMU Dependencies
    addPackageForBuild(
        'python3',
        'python3-venv',
        `python3-tomli` + backportsSuffix
    );
    addPackageForHost('libglib2.0-dev');

    // AppImage Dependencies
    addPackageForBuild(
        'appstream',
        'zsync'
    );

    // Download AppImageTool
    function getAppImageArch() {
        switch (process.arch) {
            case 'x64': return 'x86_64';
            case 'arm64': return 'aarch64';
            case 'arm': return 'armhf';
            default: err('Unsupported Build Architecture');
        }
    }
    let appimagetool = '/opt/appimagetool';
    run(['wget', '-O', appimagetool, `https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${getAppImageArch()}.AppImage`]);
    makeExecutable(appimagetool);
    run([getScriptsDir() + '/fix-appimage-for-docker.sh', appimagetool]);
    const script = `#!/bin/sh\nexec ${appimagetool} --appimage-extract-and-run "$@"\n`;
    appimagetool = '/usr/local/bin/' + path.basename(appimagetool);
    fs.writeFileSync(appimagetool, script);
    makeExecutable(appimagetool);
});

// Testing Dependencies
handlers.set(Modes.Test, function () {
    let glib = 'libglib2.0-0';
    const newerGlib = glib + 't64';
    if (doesPackageExist(newerGlib)) {
        glib = newerGlib;
    }
    addPackageForHost(
        'libc6',
        'libstdc++6',
        'libopenal1',
        glib
    );
});

// SDK Usage Dependencies
handlers.set(Modes.SDK, function () {
    addPackageForBuild(
        'cmake' + backportsSuffix,
        'ninja-build',
        'g++-arm-linux-gnueabihf',
        'gcc-arm-linux-gnueabihf'
    );
});

// Run
handlers.get(options.mode)();

// Install Packages
run(['apt-get', 'install', '--no-install-recommends', '-y', ...packages]);