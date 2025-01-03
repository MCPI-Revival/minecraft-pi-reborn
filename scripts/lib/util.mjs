import * as child_process from 'node:child_process';

// Logging
const EXIT_FAILURE = 1;
export function fail(message) {
    console.error(message);
    process.exit(EXIT_FAILURE);
}
export function err(message) {
    fail('ERROR: ' + message);
}
export function info(message) {
    console.log('INFO: ' + message);
}

// Sub-Process
export function run(command) {
    try {
        info('Running: ' + command.join(' '));
        child_process.execFileSync(command[0], command.slice(1), {stdio: 'inherit'});
    } catch (e) {
        err(e);
    }
}