import * as path from 'node:path';
import * as url from 'node:url';
import * as fs from 'node:fs';
import * as child_process from 'node:child_process';
import { err, info } from '../../lib/util.mjs';

// Tea Information
const TEA_VERSION = '0.11.1';
const TEA_URL = `https://dl.gitea.com/tea/${TEA_VERSION}/tea-${TEA_VERSION}-linux-amd64`;

// Download Tea (If Needed)
/**
 * @returns {Promise<string>}
 */
async function getTea() {
    const __filename = url.fileURLToPath(import.meta.url);
    const dir = path.dirname(__filename);
    const bin = path.join(dir, '.tea-' + TEA_VERSION);
    if (!fs.existsSync(bin)) {
        info('Downloading Tea...');
        const response = await fetch(TEA_URL);
        const buffer = await response.arrayBuffer();
        fs.writeFileSync(bin, Buffer.from(buffer));
        fs.chmodSync(bin, 0o755);
    }
    return bin;
}

// Run Tea
/**
 * @param {boolean} dryRun
 * @param {boolean} canFail Whether The Command Is Allowed To Fail
 * @param {string[]} command
 * @returns {Promise<void>}
 */
export async function tea(dryRun, canFail, command) {
    const bin = await getTea();
    command.unshift(bin);
    try {
        const prettyCommand = command
            .map(part => String(part))
            .map(part => part.includes('\n') ? '...' : part)
            .map(part => part.includes(' ') ? JSON.stringify(part) : part)
            .join(' ');
        if (!dryRun) {
            info('Running: ' + prettyCommand);
            child_process.execFileSync(command[0], command.slice(1), {stdio: 'inherit'});
        } else {
            info('Would Run: ' + prettyCommand);
        }
    } catch (e) {
        if (!canFail) {
            err(e);
        }
    }
}