#!/usr/bin/env node
import * as path from 'node:path';
import * as fs from 'node:fs';
import { parseOptions, Enum } from '../lib/options.mjs';
import { err, getScriptsDir, getVersion, run } from '../lib/util.mjs';
import { ArchitecturesMinusHost } from '../lib/options.mjs';

// Options
const Modes = new Enum([
    'Client',
    'Server'
]);
const options = parseOptions([
    ['mode', Modes],
    ['architecture', ArchitecturesMinusHost]
], [], null);

// Prepare AppImage
const scripts = getScriptsDir();
const appimage = path.join(scripts, '..', 'out', `minecraft-pi-reborn-${getVersion().data}-${options.architecture.name}.AppImage`);
if (!fs.existsSync(appimage)) {
    err('Missing AppImage!');
}
run([path.join(scripts, 'misc', 'fix-appimage-for-docker.sh'), appimage]);

// Make Test Directory
const tmp = path.join(scripts, '..', '.testing-tmp');
fs.rmSync(tmp, {recursive: true, force: true});
fs.mkdirSync(tmp);
process.env.MCPI_PROFILE_DIRECTORY = tmp;

// Run
const args = [appimage, '--appimage-extract-and-run'];
if (options.mode === Modes.Client) {
    args.push(
        '--default',
        '--no-cache',
        '--benchmark',
        '--force-headless'
    );
} else {
    args.push(
        '--server',
        '--only-generate'
    );
}
run(args);