#!/usr/bin/env node
import * as path from 'node:path';
import { run, info, getBuildToolsDir, createDir, err } from './lib/util.mjs';
import { parseOptions, createEnum, Architectures, PositionalArg } from './lib/options.mjs';
import {
    addPackageForBuild,
    addPackageForHost,
    doesPackageExist,
    getPackageForHost,
    installPackages,
    setupApt
} from './lib/apt.mjs';
import { installMsys2, installMsys2Packages } from './lib/msys2.mjs';

// Options
const Modes = {
    Build: null,
    Test: null,
    SDK: null,
    Lint: null
};
createEnum(Modes);
const options = {
    mode: PositionalArg(0, Modes),
    architecture: PositionalArg(1, Architectures)
};
parseOptions(options, null);

// Setup
setupApt(options.architecture);

// Build Dependencies
const handlers = new Map();
handlers.set(Modes.Build, function () {
    // Build Dependencies
    addPackageForBuild(
        'git',
        'cmake',
        // For Building Ninja
        'ninja-build',
        're2c',
        // For Building AppStream
        'meson',
        'libyaml-dev',
        'liblzma-dev',
        'libzstd-dev',
        'libcurl4-openssl-dev',
        'libglib2.0-dev',
        'gperf',
        // Doxygen
        'doxygen',
        'graphviz'
    );

    // Select Platform
    if (options.architecture !== Architectures.Windows) {
        // Linux Target

        // Compiler
        if (options.architecture === Architectures.Host) {
            addPackageForBuild('gcc', 'g++');
        } else {
            addPackageForBuild('crossbuild-essential-' + options.architecture.name);
        }

        // OpenAL Dependencies
        addPackageForHost('libpulse-dev');

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
    } else {
        // Windows Target

        // MSYS2 Dependencies
        addPackageForBuild(
            // https://github.com/HolyBlackCat/quasi-msys2?tab=readme-ov-file#usage
            'make',
            'wget',
            'tar',
            'zstd',
            'gawk',
            'gpg',
            'gpgv',
            // Compiler
            'clang',
            'lld',
            'llvm',
            // Compiler (For QEMU)
            'gcc',
            'g++',
            // For Generating The ICO File
            'imagemagick'
        );
    }

    // QEMU Dependencies
    addPackageForBuild(
        'python3',
        'python3-venv',
        'python3-tomli'
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

    // Setup MSYS2
    if (options.architecture === Architectures.Windows) {
        installMsys2();
        installMsys2Packages(['gcc']);
    }
});

// Testing Dependencies
function blockWindows() {
    // These modes do not make sense when targeting Windows.
    if (options.architecture === Architectures.Windows) {
        err('Windows Not Supported');
    }
}
const addTestPackages = () => {
    blockWindows();
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
};
handlers.set(Modes.Test, function () {
    addTestPackages();
    installPackages();
});

// SDK Usage Dependencies
handlers.set(Modes.SDK, function () {
    addTestPackages(); // Needed So SDK Can Be Extracted
    addPackageForBuild(
        'cmake',
        'ninja-build',
        'g++-arm-linux-gnueabihf',
        'gcc-arm-linux-gnueabihf'
    );
    installPackages();
});

// Linting Dependencies
handlers.set(Modes.Lint, function () {
    // Check System
    blockWindows();
    const nodeVersion = process.versions.node.split('.').map(Number);
    const requiredNodeVersion = 20;
    if (nodeVersion[0] < requiredNodeVersion) {
        err(`Node.js ${requiredNodeVersion}+ Required`);
    }
    // Install Packages
    addPackageForBuild(
        'shellcheck',
        'devscripts',
        'nodejs',
        'npm'
    );
    installPackages();
});

// Run
info('Installing Dependencies For: ' + options.mode.prettyName);
handlers.get(options.mode)();