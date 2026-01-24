import * as path from 'node:path';
import * as child_process from 'node:child_process';
import { tea } from './tea.mjs';
import { getFiles, getOutputDir, getToken, SERVER, SLUG, VERSION } from './common.mjs';
import { err, getScriptsDir, info } from '../../lib/util.mjs';

// Login
/**
 * @param {boolean} dryRun
 * @returns {Promise<void>}
 */
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
/**
 * @param {string} tag
 * @returns {string}
 */
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

// Create Release
/**
 * @param {boolean} dryRun
 * @param {string} tag
 * @returns {Promise<void>}
 */
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
    const files = getFiles(getOutputDir());
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