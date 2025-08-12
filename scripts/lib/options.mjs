import * as path from 'node:path';
import { fail } from './util.mjs';

// Enums
export function createEnum(obj) {
    // Setup Values
    const keys = Object.keys(obj);
    obj.values = [];
    for (const key of keys) {
        // Create Object
        const value = {
            prettyName: key,
            name: key.toLowerCase()
        };
        // Attach To Enum
        obj[key] = value;
        obj.values.push(value);
    }

    // Add Parsing Function
    obj.get = name => {
        if (name) {
            name = name.toLowerCase();
            for (const value of obj.values) {
                if (value.name === name) {
                    return value;
                }
            }
        }
        return null;
    };
}

// Supported Architectures
export const ArchitecturesMinusHost = {
    AMD64: null,
    ARM64: null,
    ARMHF: null
};
export const Architectures = {
    ...ArchitecturesMinusHost,
    Host: null
};
createEnum(ArchitecturesMinusHost);
createEnum(Architectures);

// Options Types
export const Flag = {};
function getFlags(options) {
    return Object.keys(options)
        .filter(key => options[key] === Flag)
        .sort();
}
export function PositionalArg(i, obj) {
    return {i, obj};
}
function getPositionalArguments(options) {
    const flags = getFlags(options);
    return Object.keys(options)
        .filter(key => !flags.includes(key))
        .map(key => ({ key, ...options[key] }))
        .sort((a, b) => a.i - b.i);
}

// Usage Text
function formatFlag(name) {
    return '--' + name;
}
function formatOptionalArg(arg) {
    return '[' + arg + '] ';
}
function getUsageText(options, customHandler) {
    // Usage Text
    const exe = path.basename(process.argv[1]);
    let usage = `USAGE: ${exe} `;

    // Positional Arguments
    const positionalArgs = getPositionalArguments(options);
    for (const arg of positionalArgs) {
        const arr = [];
        for (const value of arg.obj.values) {
            arr.push(value.name);
        }
        usage += '<' + arr.join('|') + '> ';
    }

    // Flags
    const flags = getFlags(options);
    for (const flag of flags) {
        usage += formatOptionalArg(formatFlag(flag));
    }

    // Custom Arguments
    if (customHandler) {
        usage += formatOptionalArg(customHandler(null));
    }

    // Return
    return usage.trim();
}

// Parse
export function parseOptions(options, customHandler) {
    // Prepare
    const usage = getUsageText(options, customHandler);
    const positionalArgs = getPositionalArguments(options);
    const flags = getFlags(options);

    // Copy Arguments
    const args = process.argv.slice(2); // Skip First Two Arguments

    // Read Positional Arguments
    for (const arg of positionalArgs) {
        // Parse
        let value = args.shift();
        value = arg.obj.get(value);
        // Set
        if (value === null) {
            fail(usage);
        }
        options[arg.key] = value;
    }

    // Read Flags
    for (const flag of flags) {
        const name = formatFlag(flag);
        const isSet = args.includes(name);
        if (isSet) {
            args.splice(args.indexOf(name), 1);
        }
        options[flag] = isSet;
    }

    // Unknown Arguments
    for (const arg of args) {
        if (!customHandler || !customHandler(arg)) {
            fail(usage);
        }
    }
}