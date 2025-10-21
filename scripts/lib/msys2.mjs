import path from 'node:path';
import fs from 'node:fs';
import * as child_process from 'node:child_process';
import { err, getScriptsDir, info, run } from './util.mjs';

// Root Directory
export function getMsys2Root() {
    const __dirname = getScriptsDir();
    const root = path.join(__dirname, '..');
    return path.join(root, 'quasi-msys2');
}

// Install
const varsScript = path.join('env', 'vars.src');
export function installMsys2() {
    const dir = getMsys2Root();
    fs.rmSync(dir, {recursive: true, force: true});
    run(['git', 'clone', 'https://github.com/HolyBlackCat/quasi-msys2.git', dir]);
    // Patch CMake Flags
    const file = path.join(dir, varsScript);
    let data = fs.readFileSync(file, 'utf8');
    data = data.replace(' -DCMAKE_INSTALL_PREFIX=$MSYSTEM_PREFIX', '');
    fs.writeFileSync(file, data);
}
export function installMsys2Packages(packages) {
    const cmd = ['make', '-C', getMsys2Root(), 'install'];
    for (const pkg of packages) {
        cmd.push(`_${pkg}`);
    }
    run(cmd);
}

// Apply Environment
export function enableMsys2() {
    info('Enabling MSYS2...');
    const script = 'source ./env/vars.src > /dev/null && printenv';
    const command = ['bash', '-c', script];
    const dir = getMsys2Root();
    try {
        // Get Variables
        const env = child_process.execFileSync(command[0], command.slice(1), {
            stdio: ['ignore', 'pipe', 'inherit'],
            cwd: dir,
            encoding: 'utf8'
        });
        // Set Variables
        for (const line of env.split('\n')) {
            const index = line.indexOf('=');
            if (index !== -1) {
                const key = line.substring(0, index);
                process.env[key] = line.substring(index + 1);
            }
        }
    } catch (e) {
        err('Unable To Enable MSYS2');
    }
}