import * as path from 'node:path';
import * as fs from 'node:fs';
import { err, getScriptsDir, getVersion, info } from '../../lib/util.mjs';

// Repository Information
export const GIT_TAG_PREFIX = 'refs/tags/';
export const SERVER = 'https://gitea.thebrokenrail.com';
export const ORGANIZATION = 'minecraft-pi-reborn';
export const REPOSITORY = 'minecraft-pi-reborn';
export const SLUG = ORGANIZATION + '/' + REPOSITORY;
export const VERSION = getVersion().data;

// Debian Packaging Information
export const DEBIAN_COMPONENT = 'main';
export const DEBIAN_STABLE_DISTRIBUTION = 'stable';
export const DEBIAN_UNSTABLE_DISTRIBUTION = 'unstable';
export const DEBIAN_EXTENSION = '.deb';

// Discord Invite URL
process.env.DISCORD_URL = 'https://discord.gg/HAErXkUkpR';

// Get Release Token
/**
 * @returns {string}
 */
export function getToken() {
    const token = process.env.RELEASE_TOKEN;
    if (!token && !getToken.ignoreMissing) {
        err('Missing Release Token');
    }
    return token;
}
getToken.ignoreMissing = false;

// Get Output Directory
/**
 * @returns {string}
 */
export function getOutputDir() {
    const scripts = getScriptsDir();
    const root = path.join(scripts, '..');
    const out = path.join(root, 'out');
    if (!fs.existsSync(out)) {
        err('Missing Output Directory');
    }
    return out;
}

// Safe fetch()
/**
 * @param {string} url
 * @param {RequestInit} options
 * @returns {Promise<Response>}
 */
export async function safeFetch(url, options) {
    if (safeFetch.dryRun) {
        info(`Would Send ${options.method} Request To: ${url}`);
        return new Response();
    }
    return fetch(url, options);
}
safeFetch.dryRun = false;

// Get Files Recursively
/**
 * @param {string} dir
 * @param {number} level The Recursion Limit
 * @returns {string[]}
 */
export function getFiles(dir, level = 1) {
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