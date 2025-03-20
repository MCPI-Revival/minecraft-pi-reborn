#!/usr/bin/env node
import * as path from 'node:path';
import * as fs from 'node:fs';
import { err, run, getDebianVersion, info, doesPackageExist, getBuildToolsDir, createDir } from './lib/util.mjs';
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
    // This will install packages thet match
    // the build machine's architecture.
    // This is usually used for build tools.
    packages.push(...arr);
}
function getPackageForHost(name) {
    return name + archSuffix;
}
function addPackageForHost(...arr) {
    // This will install packages thet match
    // the host machine's architecture.
    // This is usually used for libraries.
    for (const name of arr) {
        packages.push(getPackageForHost(name));
    }
}
function installPackages() {
    // Install Queued Packages
    run(['apt-get', 'install', '--no-install-recommends', '-y', ...packages]);
}

// Build Dependencies
const handlers = new Map();
handlers.set(Modes.Build, function () {
    // Build Dependencies
    addPackageForBuild(
        'git',
        'cmake' + backportsSuffix,
        // For Building Ninja
        'ninja-build',
        're2c',
        // For Building AppStream
        'libyaml-dev',
        'libxmlb-dev',
        'liblzma-dev',
        'libcurl4-openssl-dev',
        'libglib2.0-dev',
        'meson',
        'gperf'
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
    addPackageForBuild('zsync');

    // Install Packages
    installPackages();

    // Build Tools
    const buildToolsDir = getBuildToolsDir();
    const buildDir = path.join(buildToolsDir, 'build');
    createDir(buildDir, false);
    run([
        'cmake',
        '-GNinja',
        '-S', buildToolsDir, '-B', buildDir
    ]);
    run(['cmake', '--build', buildDir]);
    run(['cmake', '--install', buildDir]);
});

// Testing Dependencies
handlers.set(Modes.Test, function () {
    let glib = 'libglib2.0-0';
    const newerGlib = glib + 't64';
    if (doesPackageExist(getPackageForHost(newerGlib))) {
        glib = newerGlib;
    }
    addPackageForHost(
        'libc6',
        'libstdc++6',
        'libopenal1',
        glib
    );
    installPackages();
});

// SDK Usage Dependencies
handlers.set(Modes.SDK, function () {
    addPackageForBuild(
        'cmake' + backportsSuffix,
        'ninja-build',
        'g++-arm-linux-gnueabihf',
        'gcc-arm-linux-gnueabihf'
    );
    installPackages();
});

// Run
handlers.get(options.mode)();