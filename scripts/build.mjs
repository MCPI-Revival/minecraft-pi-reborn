#!/usr/bin/env node
import * as path from 'node:path';
import * as url from 'node:url';
import * as fs from 'node:fs';
import * as child_process from 'node:child_process';

// Logging
const EXIT_FAILURE = 1;
function fail(message) {
    console.error(message);
    process.exit(EXIT_FAILURE);
}
function err(message) {
    fail('ERROR: ' + message);
}
function info(message) {
    console.log('INFO: ' + message);
}

// Enums
function Enum(values) {
    for (const value of values) {
        this[value] = {
            prettyName: value,
            name: value.toLowerCase()
        };
    }
}
Enum.prototype.get = function (name) {
    for (const value in this) {
        if (value.toLowerCase() === name.toLowerCase()) {
            return this[value];
        }
    }
    return null;
};
function wrap(obj) {
    return new Proxy(obj, {
        get(target, property) {
            if (property in target) {
                return target[property];
            } else {
                err('Undefined Value: ' + property);
            }
        }
    });
}
const PackageTypes = wrap(new Enum([
    'None',
    'AppImage',
    'Flatpak'
]));
const Architectures = wrap(new Enum([
    'AMD64',
    'ARM64',
    'ARMHF',
    'Host'
]));
const Configurations = wrap(new Enum([
    'Release',
    'Debug'
]));

// Folders
const __filename = url.fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const root = path.join(__dirname, '..');
let build = path.join(root, 'build');
let out = path.join(root, 'out');

// Positional Arguments
let argIndex = 2; // Skip First Two Arguments
function parseArg(arg, from, type) {
    // Check Argument
    if (arg === undefined) {
        err('Expecting ' + type);
    }
    // Read Argument
    const value = from.get(arg);
    if (value === null) {
        err(`Invalid ${type}: ${arg}`);
    }
    // Return
    return value;
}
function readArg(...args) {
    return parseArg(process.argv[argIndex++], ...args);
}
// Type Of Packaging
const packageType = readArg(PackageTypes, 'Package Type');
// Build Architecture
const architecture = readArg(Architectures, 'Architecture');
// Flatpak Builds Work Best Without Custom Toolchains
if (packageType === PackageTypes.Flatpak && architecture !== Architectures.Host) {
    err('Flatpak Builds Do Not Support Custom Toolchains');
}

// CMake Build Options
const options = new Map();

// Other Arguments
let clean = false;
let install = false;
let config = Configurations.Release;
for (; argIndex < process.argv.length; argIndex++) {
    const arg = process.argv[argIndex];
    const cmakeArgPrefix = '-D';
    if (arg.startsWith(cmakeArgPrefix)) {
        // Pass Build Option To CMake
        let parsedArg = arg.substring(cmakeArgPrefix.length);
        const parts = parsedArg.split('=');
        if (parts.length !== 2) {
            err('Unable To Parse Build Option: ' + arg);
        }
        const name = parts[0];
        const value = parts[1];
        if (!/^[a-zA-Z_]+$/.test(name) || name.length === 0) {
            err('Invalid Build Option Name: ' + name);
        }
        options.set(name, value);
    } else if (arg === '--clean') {
        // Remove Existing Build Directory
        clean = true;
    } else if (arg === '--install') {
        // Install To System Instead Of Output Directory
        if (packageType === PackageTypes.AppImage) {
            err('AppImages Cannot Be Installed');
        }
        install = true;
    } else if (arg === '--config') {
        // Set Configuration
        config = parseArg(process.argv[++argIndex], Configurations, 'Configuration');
    } else {
        // Invalid
        err('Invalid Argument: ' + arg);
    }
}

// Update Folders
function updateDir(dir) {
    if (packageType !== PackageTypes.None) {
        dir = path.join(dir, packageType.name);
    }
    return path.join(dir, architecture.name);
}
build = updateDir(build);
let cleanOut = false;
// AppImages Are Placed Directly In ./out
if (packageType !== PackageTypes.AppImage) {
    cleanOut = true;
    out = updateDir(out);
}

// Configure Build Options
function toCmakeBool(val) {
    return val ? 'ON' : 'OFF';
}
options.set('MCPI_IS_APPIMAGE_BUILD', toCmakeBool(packageType === PackageTypes.AppImage));
options.set('MCPI_IS_FLATPAK_BUILD', toCmakeBool(packageType === PackageTypes.Flatpak));
if (architecture !== Architectures.Host) {
    options.set('CMAKE_TOOLCHAIN_FILE', path.join(root, 'cmake', 'toolchain', architecture.name + '-toolchain.cmake'));
} else {
    options.delete('CMAKE_TOOLCHAIN_FILE');
}

// Make Build Directory
function createDir(dir, clean) {
    if (clean) {
        fs.rmSync(dir, {recursive: true, force: true});
    }
    fs.mkdirSync(dir, {recursive: true});
}
createDir(build, clean);
if (!install) {
    createDir(out, cleanOut);
}

// Run CMake
function run(command) {
    try {
        info('Running: ' + command.join(' '));
        child_process.execFileSync(command[0], command.slice(1), {cwd: build, stdio: 'inherit'});
    } catch (e) {
        err(e);
    }
}
const configure = ['cmake', '-GNinja Multi-Config'];
options.forEach((value, key, map) => {
    configure.push(`-D${key}=${value}`);
});
configure.push(root);
run(configure);

// Build
const configArg = ['--config', config.prettyName];
run(['cmake', '--build', '.', ...configArg/*, '-v'/*, '--', '-n', '-j1', '-j1'*/]);

// Package
if (packageType !== PackageTypes.AppImage) {
    if (!install) {
        process.env.DESTDIR = out;
    }
    run(['cmake', '--install', '.', ...configArg]);
} else {
    run(['cmake', '--build', '.', '--target', 'package', ...configArg]);
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
