import * as path from 'node:path';
import { fail } from './util.mjs';

// Type Definitions
/**
 * @typedef {object} EnumValue
 * @property {string} prettyName
 * @property {string} name
 */
/**
 * @typedef {object} EnumBase
 * @property {EnumValue[]} values
 * @property {function(string | undefined): EnumValue | null} get
 */
/**
 * @template {Readonly<{[key: string]: null}>} T
 * @typedef {Readonly<{[K in keyof T]: EnumValue} & {raw: T} & EnumBase>} Enum
 */

// Enums
/**
 * @template T
 * @param {T} obj
 * @returns {Enum<T>}
 */
export function createEnum(obj) {
    // Setup Values
    /** @type {{[K in keyof T]?: EnumValue}} */
    const map = {};
    /** @type {EnumValue[]} */
    const values = [];
    for (const key of Object.keys(obj)) {
        // Create Object
        /** @type {EnumValue} */
        const value = {
            prettyName: key,
            name: key.toLowerCase()
        };
        // Attach To Enum
        map[key] = value;
        values.push(value);
    }

    // Add Parsing Function
    /**
     * @param {string | undefined} name
     * @returns {EnumValue | null}
     */
    const get = (name) => {
        if (name) {
            name = name.toLowerCase();
            for (const value of values) {
                if (value.name === name) {
                    return value;
                }
            }
        }
        return null;
    };

    // Return
    return {
        ...map,
        values,
        raw: obj,
        get
    };
}

// Supported Architectures
export const ArchitecturesMinusHost = createEnum({
    AMD64: null,
    ARM64: null,
    ARMHF: null
});
export const Architectures = createEnum({
    ...ArchitecturesMinusHost.raw,
    Host: null,
    Windows: null
});
/**
 * @param {EnumValue} architecture
 * @returns {EnumValue}
 */
export function convertArchitectureToLinux(architecture) {
    if (architecture === Architectures.Windows) {
        return Architectures.AMD64;
    }
    return architecture;
}

// Options Types
/**
 * @class
 */
export function Flag() {
}
/**
 * @class
 * @property {number} i
 * @property {EnumBase} obj
 */
export function PositionalArg(i, obj) {
    this.i = i;
    this.obj = obj;
}
/**
 * @typedef {Flag | PositionalArg} OptionsTemplateEntry
 */
/**
 * @typedef {Record<string, OptionsTemplateEntry>} OptionsTemplate
 */
/**
 * @param {OptionsTemplate} options
 * @returns {{flags: string[], positionalArgs: {key: string, arg: PositionalArg}[]}}
 */
function splitOptionsTemplate(options) {
    // Get Entries
    /** @type {{key: string, arg: OptionsTemplateEntry}[]} */
    const entries = Object.keys(options)
        .map(key => ({key, arg: options[key]}));
    // Split
    const out = {
        /** @type {string[]} */
        flags: [],
        /** @type {{key: string, arg: PositionalArg}[]} */
        positionalArgs: []
    };
    for (const entry of entries) {
        const arg = entry.arg;
        if (arg instanceof Flag) {
            out.flags.push(entry.key);
        } else {
            out.positionalArgs.push({key: entry.key, arg});
        }
    }
    return out;
}

// Usage Text
/**
 * @param {string} name
 * @returns {string}
 */
function formatFlag(name) {
    return `--${name}`;
}
/**
 * @param {string} arg
 * @returns {string}
 */
function formatOptionalArg(arg) {
    return `[${arg}]`;
}
/**
 * @param {OptionsTemplate} options
 * @param {function(null): string} customHandler
 * @returns {string}
 */
function getUsageText(options, customHandler) {
    // Usage Text
    const exe = path.basename(process.argv[1]);
    const separator = '\n    ';
    let usage = `USAGE: ${exe}${separator}`;

    // Get All Possible Options
    const args = splitOptionsTemplate(options);

    // Positional Arguments
    const positionalArgs = args.positionalArgs
        .map(obj => ({key: obj.key, ...obj.arg}))
        .sort((a, b) => a.i - b.i);
    for (const arg of positionalArgs) {
        const arr = [];
        for (const value of arg.obj.values) {
            arr.push(value.name);
        }
        usage += `<${arr.join('|')}>${separator}`;
    }

    // Flags
    const flags = args.flags
        .sort();
    for (const flag of flags) {
        usage += formatOptionalArg(formatFlag(flag)) + separator;
    }

    // Custom Arguments
    if (customHandler) {
        usage += formatOptionalArg(customHandler(null));
    }

    // Return
    return usage.trim();
}

// Parse
/**
 * @template {OptionsTemplateEntry} T
 * @typedef {T extends PositionalArg ? EnumValue : boolean} OptionsEntry
 */
/**
 * @template {Readonly<OptionsTemplate>} T
 * @typedef {Readonly<{[K in keyof T]: OptionsEntry<T[K]>}>} Options
 */
/**
 * @template {Readonly<OptionsTemplate>} T
 * @param {T} options
 * @param {function(string | null): boolean | string} customHandler
 * @returns {Options<T>}
 */
export function parseOptions(options, customHandler) {
    // Prepare
    const usage = getUsageText(options, customHandler);
    const args = process.argv.slice(2); // Skip First Two Arguments
    /** @type {Record<string, boolean | EnumValue>} */
    const out = {};

    // Read Positional Arguments
    let amountOfPositionalArgs = 0;
    for (const key in options) {
        const arg = options[key];
        if (arg instanceof PositionalArg) {
            // Parse
            const str = args[arg.i];
            const value = arg.obj.get(str);
            // Set
            if (value === null) {
                fail(usage);
            }
            out[key] = value;
            amountOfPositionalArgs++;
        }
    }
    args.splice(0, amountOfPositionalArgs);

    // Read Flags
    for (const key in options) {
        const arg = options[key];
        if (arg instanceof Flag) {
            // Determine If Present
            const name = formatFlag(key);
            const index = args.indexOf(name);
            // Set
            const isSet = index !== -1;
            if (isSet) {
                args.splice(index, 1);
            }
            out[key] = isSet;
        }
    }

    // Unknown Arguments
    for (const arg of args) {
        if (!customHandler || !customHandler(arg)) {
            fail(usage);
        }
    }

    // Return
    return /** @type {Options<T>} */ (out);
}