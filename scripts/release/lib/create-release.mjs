import * as path from 'node:path';
import * as fs from 'node:fs';
import * as child_process from 'node:child_process';
import { tea } from './tea.mjs';
import { getOutputDir, getToken, SERVER, SLUG, VERSION } from './common.mjs';
import { err, getScriptsDir, info } from '../../lib/util.mjs';

// Login
async function login(dryRun) {
    const name = 'ci';
    await tea(dryRun, true, ['logins', 'delete', name]);
    await tea(dryRun, false, [
        'logins', 'add',
        '--name', name,
        '--url', SERVER,
        '--token', getToken()
    ]);
    await tea(dryRun, false, ['logins', 'default', name]);
}

// Get Changelog
function getChangelog(tag) {
    process.env.URL = `${SERVER}/${SLUG}/src/tag/${tag}`;
    const script = path.join(getScriptsDir(), 'misc', 'get-changelog.mjs');
    try {
        return child_process.execFileSync(script, ['release'], {
            stdio: ['ignore', 'pipe', 'inherit'],
            encoding: 'utf8'
        }).trim();
    } catch (e) {
        err('Unable To Retrieve Changelog');
    }
}

// Get Files Recursively
function getFiles(dir, level) {
    const out = [];
    const files = fs.readdirSync(dir);
    for (const file of files) {
        const full = path.join(dir, file);
        if (fs.statSync(full).isDirectory()) {
            if (level > 0) {
                out.push(...getFiles(full, level - 1));
            }
        } else {
            out.push(full);
        }
    }
    return out;
}

// Create Release
export async function createRelease(dryRun, tag) {
    // Get Changelog
    const changelog = getChangelog(tag);

    // Create Release
    await login(dryRun);
    info('Creating Release: ' + tag);
    await tea(dryRun, true, [
        'releases', 'delete',
        '--repo', SLUG,
        tag
    ]);
    await tea(dryRun, false, [
        'releases', 'create',
        '--repo', SLUG,
        '--tag', tag,
        '--title', `v${VERSION}`,
        '--note', changelog
    ]);

    // Attach Files
    const files = getFiles(getOutputDir(), 1);
    for (const file of files) {
        if (file.endsWith('.so')) {
            // Skip Example Mods
            continue;
        }
        // Upload
        info('Attaching File: ' + path.basename(file));
        await tea(dryRun, false, [
            'releases', 'assets', 'create',
            '--repo', SLUG,
            tag,
            file
        ]);
    }
}