#!/usr/bin/env node
import * as path from 'node:path';
import * as fs from 'node:fs';
import { getScriptsDir } from './lib/util.mjs';
import { Enum, parseOptions } from './lib/options.mjs';

// Parse Options
const Modes = new Enum([
    'CI',
    'AppStream'
]);
const options = parseOptions([['mode', Modes]], [], null);

// Read Files
function readFile(...args) {
    const __dirname = getScriptsDir();
    const root = path.join(__dirname, '..');
    return {
        data: fs.readFileSync(path.join(root, ...args), 'utf8').trim(),
        time: fs.statSync(path.join(root, ...args)).mtime
    };
}
const version = readFile('VERSION');
const changelog = readFile('docs', 'CHANGELOG.md');

// Print Version
if (options.mode === Modes.CI) {
    console.log('version=' + version.data);
} else {
    const time = new Date(Math.max(version.time.getTime(), changelog.time.getTime()));
    const date = time.toISOString().split('T')[0];
    console.log(`<release version="${version.data}" date="${date}">`);
    console.log('<url>' + process.env.URL + '</url>');
}

// Parse Changelog
const out = [];
let foundStart = false;
const lines = changelog.data.split('\n');
for (const line of lines) {
    if (!foundStart) {
        // Found Start Of Version Info
        foundStart = line.trim() === '## ' + version.data;
    } else if (line.trim().length === 0) {
        // Found End
        break;
    } else {
        // Found Entry
        out.push(line);
    }
}

// Print
if (options.mode === Modes.CI) {
    const delimiter = 'CHANGELOG_EOF';
    console.log('changelog<<' + delimiter);
    console.log(out.join('\n'));
    console.log(delimiter);
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
            console.log('<li>' + line + '</li>');
        }
    }
    console.log('</ul>');
    console.log('</description>');
    console.log('</release>');
}