#!/usr/bin/env node
import { getVersion, readFile } from '../lib/util.mjs';
import { createEnum, parseOptions, PositionalArg } from '../lib/options.mjs';

// Parse Options
const Modes = {
    Release: null,
    AppStream: null,
    Date: null
};
createEnum(Modes);
const options = {
    mode: PositionalArg(0, Modes)
};
parseOptions(options, null);

// Read Files
const version = getVersion();
const changelog = readFile('docs', 'CHANGELOG.md');

// Get Date
const time = new Date(Math.max(version.time.getTime(), changelog.time.getTime()));
const date = time.toISOString().split('T')[0];
if (options.mode === Modes.Date) {
    console.log(date);
    process.exit();
}

// Print Version
const url = process.env.URL;
if (options.mode === Modes.AppStream) {
    console.log(`<release version="${version.data}" date="${date}">`);
    console.log(`<url>${url}</url>`);
}

// Parse Changelog
const out = [];
let foundStart = false;
let skipLine = false;
const lines = changelog.data.split('\n');
for (const line of lines) {
    if (!foundStart) {
        // Found Start Of Version Info
        foundStart = line.trim() === '## ' + version.data;
        skipLine = true;
    } else if (skipLine) {
        // Skip This Line
        skipLine = false;
    } else if (line.trim().length === 0) {
        // Found End
        break;
    } else {
        // Found Entry
        out.push(line);
    }
}

// Add Header
if (options.mode === Modes.Release) {
    const discord = process.env.DISCORD_URL;
    out.unshift(
        '## Links',
        `* [Installation Guide](${url}/docs/INSTALL.md)`,
        `* [Getting Started Guide](${url}/docs/GETTING_STARTED.md)`,
        `* [Documentation](${url}/docs)`,
        `* [Discord](${discord})`,
        '',
        '## Changelog'
    );
}

// Print
if (options.mode === Modes.Release) {
    console.log(out.join('\n'));
} else {
    console.log('<description>');
    console.log('<ul>');
    const prefix = '* '; // AppStream Does Not Support Nested Lists
    for (let line of out) {
        if (line.startsWith(prefix)) {
            line = line.substring(prefix.length).trim();
            const code = '`';
            while (line.includes(code)) {
                line = line.replace(code, '<code>');
                line = line.replace(code, '</code>');
            }
            // Output
            console.log(`<li>${line}</li>`);
        }
    }
    console.log('</ul>');
    console.log('</description>');
    console.log('</release>');
}