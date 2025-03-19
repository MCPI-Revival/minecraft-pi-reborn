#!/usr/bin/env node
import * as path from 'node:path';
import * as fs from 'node:fs';
import { info, err, run, createDir, getScriptsDir, getBuildToolsBin, getParallelFlag } from './lib/util.mjs';
import { parseOptions, Enum, Architectures } from './lib/options.mjs';

// CMake Options
const cmakeArgPrefix = '-D';
const cmakeArgSeparator = '=';
const cmakeOptions = new Map();
function parseCMakeOption(arg) {
    if (arg === null) {
        // Usage Text
        return `${cmakeArgPrefix}var${cmakeArgSeparator}value...`;
    } else if (arg.startsWith(cmakeArgPrefix)) {
        // Pass Build Option To CMake
        let parsedArg = arg.substring(cmakeArgPrefix.length);
        const parts = parsedArg.split(cmakeArgSeparator);
        if (parts.length !== 2) {
            err('Unable To Parse Build Option: ' + arg);
        }
        const name = parts[0];
        const value = parts[1];
        if (!/^[a-zA-Z_]+$/.test(name) || name.length === 0) {
            err('Invalid Build Option Name: ' + name);
        }
        cmakeOptions.set(name, value);
        return true;
    } else {
        // Unknown Option
        return false;
    }
}

// Options
const PackageTypes = new Enum([
    'None',
    'AppImage',
    'Flatpak'
]);
const options = parseOptions([
    ['packageType', PackageTypes],
    ['architecture', Architectures]
], [
    'clean',
    'install',
    'debug'
], parseCMakeOption);

// Check Options
if (options.packageType === PackageTypes.Flatpak && options.architecture !== Architectures.Host) {
    err('Flatpak Builds Do Not Support Custom Toolchains');
}
if (options.packageType === PackageTypes.AppImage && options.install) {
    err('AppImages Cannot Be Installed');
}

// Folders
const __dirname = getScriptsDir();
const root = path.join(__dirname, '..');
let build = path.join(root, 'build');
let out = path.join(root, 'out');

// Update Build Directory
function specializeDir(dir) {
    // Use Unique Folder For Build Type
    return path.join(dir, options.packageType.name, options.architecture.name);
}
build = specializeDir(build);
// Update Output Directory
const useOutRoot = options.packageType === PackageTypes.AppImage;
if (!useOutRoot) {
    out = specializeDir(out);
}
// Print Directories
function printDir(name, dir) {
    info(name + ' Directory: ' + dir);
}
printDir('Build', build);
printDir('Output', out);

// Configure CMake Options
function setupPackageTypeOption(type) {
    cmakeOptions.set(`MCPI_IS_${type.name.toUpperCase()}_BUILD`, options.packageType === type ? 'ON' : 'OFF');
}
setupPackageTypeOption(PackageTypes.AppImage);
setupPackageTypeOption(PackageTypes.Flatpak);
const toolchainOption = 'CMAKE_TOOLCHAIN_FILE';
if (options.architecture !== Architectures.Host) {
    cmakeOptions.set(toolchainOption, path.join(root, 'cmake', 'toolchain', options.architecture.name + '-toolchain.cmake'));
} else {
    cmakeOptions.delete(toolchainOption);
}

// Make Build Directory
createDir(build, options.clean);
if (!options.install) {
    createDir(out, !useOutRoot);
}

// Use Build Tools
const buildTools = getBuildToolsBin();
const makeTool = 'make';
const hasBuildTools = fs.existsSync(path.join(buildTools, makeTool));
if (hasBuildTools) {
    function prependEnv(env, value) {
        const old = process.env[env];
        if (old) {
            value += ':' + old;
        }
        process.env[env] = value;
    }
    prependEnv('PATH', getBuildToolsBin());
}

// Run CMake
const configure = ['cmake', '-GNinja Multi-Config'];
cmakeOptions.forEach((value, key) => {
    configure.push(cmakeArgPrefix + key + cmakeArgSeparator + value);
});
configure.push('-S', root, '-B', build);
run(configure);

// Build
const configArg = ['--config', options.debug ? 'Debug' : 'Release'];
if (hasBuildTools) {
    fs.writeFileSync(path.join(build, 'Makefile'), `.PHONY: all\nall:\n\t+@cmake --build . ${configArg.join(' ')}\n`);
    run([makeTool, '-C', build, getParallelFlag(), '--jobserver-style=fifo']);
} else {
    run(['cmake', '--build', build, getParallelFlag(), ...configArg]);
}

// Package
if (options.packageType !== PackageTypes.AppImage) {
    if (!options.install) {
        process.env.DESTDIR = out;
    }
    run(['cmake', '--install', build, ...configArg]);
} else {
    run(['cmake', '--build', build, '--target', 'package', ...configArg]);
    // Copy Generated Files
    const files = fs.readdirSync(build);
    for (const file of files) {
        if (file.includes('.AppImage')) {
            info('Copying: ' + file);
            const src = path.join(build, file);
            const dst = path.join(out, file);
            fs.copyFileSync(src, dst);
        }
    }
}
