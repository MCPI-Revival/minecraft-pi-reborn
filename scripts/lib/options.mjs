import { fail } from './util.mjs';

// Enums
export function Enum(values) {
    this.values = [];
    for (const value of values) {
        const obj = {
            prettyName: value,
            name: value.toLowerCase()
        };
        this[value] = obj;
        this.values.push(obj);
    }
}
Enum.prototype.get = function (name) {
    if (name) {
        for (const value of this.values) {
            if (value.name === name.toLowerCase()) {
                return value;
            }
        }
    }
    return null;
};

// Supported Architectures
export const Architectures = new Enum([
    'AMD64',
    'ARM64',
    'ARMHF',
    'Host'
]);

// Parse
function formatFlag(name) {
    return '--' + name;
}
function formatOptionalArg(arg) {
    return '[' + arg + '] ';;
}
export function parseOptions(positionalArgs, flags, customHandler) {
    // Usage Text
    let usage = 'USAGE: ';
    for (const arg of positionalArgs) {
        const option = arg[1];
        const arr = [];
        for (const value of option.values) {
            arr.push(value.name);
        }
        usage += '<' + arr.join('|') + '> ';
    }
    for (const flag of flags) {
        usage += formatOptionalArg(formatFlag(flag));
    }
    usage += formatOptionalArg(customHandler(null));
    usage = usage.trim();

    // Copy Arguments
    const args = process.argv.slice(2); // Skip First Two Arguments

    // Read Positional Arguments
    const out = {};
    for (const arg of positionalArgs) {
        let value = args.shift();
        value = arg[1].get(value);
        if (value === null) {
            fail(usage);
        }
        out[arg[0]] = value;
    }

    // Read Flags
    for (const flag of flags) {
        const name = formatFlag(flag);
        const isSet = args.includes(name);
        if (isSet) {
            args.splice(args.indexOf(name), 1);
        }
        out[flag] = isSet;
    }

    // Unknown Arguments
    for (const arg of args) {
        if (!customHandler(arg)) {
            fail(usage);
        }
    }

    // Return
    return out;
}