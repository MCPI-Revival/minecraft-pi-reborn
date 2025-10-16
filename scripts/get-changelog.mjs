#!/usr/bin/env node
import * as path from 'node:path';
import * as fs from 'node:fs';

// Read Files
function readFile(...args) {
    return fs.readFileSync(path.join(...args), 'utf8').trim();
}
const version = readFile('VERSION');
const changelog = readFile('docs', 'CHANGELOG.md');

// Print Version
console.log('version=' + version);

// Parse Changelog
const out = [];
let foundStart = false;
let skipLine = false;
const lines = changelog.split('\n');
for (const line of lines) {
    if (!foundStart) {
        // Found Start Of Version Info
        foundStart = line.includes(`## ${version}`);
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

// Print
const delimiter = 'CHANGELOG_EOF';
console.log('changelog<<' + delimiter);
console.log(out.join('\n'));
console.log(delimiter);
