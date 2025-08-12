#!/usr/bin/env node
import * as path from 'node:path';
import * as fs from 'node:fs';
import * as os from 'node:os';
import { info, err, run, createDir, getScriptsDir, getBuildToolsBin } from './lib/util.mjs';
import { parseOptions, createEnum, Architectures, Flag, PositionalArg } from './lib/options.mjs';

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
const PackageTypes = {
    None: null,
    AppImage: null,
    Flatpak: null,
    Debian: null
};
createEnum(PackageTypes);
const options = {
    // Positional Arguments
    packageType: PositionalArg(0, PackageTypes),
    architecture: PositionalArg(1, Architectures),
    // Flags
    clean: Flag,
    install: Flag,
    debug: Flag,
    verbose: Flag
};
parseOptions(options, parseCMakeOption);

// CPack
const useCPack = [PackageTypes.AppImage, PackageTypes.Debian].includes(options.packageType);
const cpackExtensions = ['.AppImage', '.deb'];

// Check Options
if (options.packageType === PackageTypes.Flatpak && options.architecture !== Architectures.Host) {
    err('Flatpak Builds Do Not Support Custom Toolchains');
}
if (useCPack && options.install) {
    err('Cannot Install When Using CPack');
}

// Folders
const __dirname = getScriptsDir();
const root = path.join(__dirname, '..');
let build = path.join(root, 'build');
const rootOut = path.join(root, 'out');
let out = rootOut;

// Update Directories
function specializeDir(dir) {
    // Use Unique Folder For Architecture
    return path.join(dir, options.architecture.name);
}
build = specializeDir(build);
out = specializeDir(out);
// Print Directories
function printDir(name, dir) {
    info(name + ' Directory: ' + dir);
}
printDir('Build', build);
printDir('Output', out);

// Configure CMake Options
cmakeOptions.set('MCPI_PACKAGING_TYPE', options.packageType.name);
const toolchainOption = 'CMAKE_TOOLCHAIN_FILE';
if (options.architecture !== Architectures.Host) {
    cmakeOptions.set(toolchainOption, path.join(root, 'cmake', 'toolchain', options.architecture.name + '-toolchain.cmake'));
} else {
    cmakeOptions.delete(toolchainOption);
}

// Make Build Directory
createDir(build, options.clean);
if (!options.install) {
    createDir(out, true);
}

// Clean Up Old CPack Packages
function handleCPackFiles(callback) {
    const files = fs.readdirSync(build);
    for (const file of files) {
        for (const extension of cpackExtensions) {
            if (file.includes(extension)) {
                callback(file);
                break;
            }
        }
    }
}
handleCPackFiles(file => {
    file = path.join(build, file);
    fs.unlinkSync(file);
});

// Use Build Tools
const buildTools = getBuildToolsBin();
const jobserver = 'jobserver_pool.py';
const supportsJobserver = fs.existsSync(path.join(buildTools, jobserver));
if (supportsJobserver) {
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
const buildCommand = ['cmake', '--build', build, ...configArg];
if (options.verbose) {
    buildCommand.push('-v');
}
if (supportsJobserver) {
    buildCommand.unshift(jobserver, '--fifo', path.join(os.tmpdir(), '.jobserver-' + process.pid));
}
run(buildCommand);

// Package
if (!useCPack) {
    if (!options.install) {
        process.env.DESTDIR = out;
    }
    run(['cmake', '--install', build, ...configArg]);
} else {
    run(['cmake', '--build', build, '--target', 'package', ...configArg]);
    // Copy Generated Files
    handleCPackFiles(file => {
        info('Copying: ' + file);
        const src = path.join(build, file);
        const dst = path.join(rootOut, file);
        fs.copyFileSync(src, dst);
    });
}
