import * as child_process from 'node:child_process';
import * as fs from 'node:fs';
import * as path from 'node:path';
import {
    DEBIAN_COMPONENT,
    DEBIAN_EXTENSION,
    getOutputDir,
    getToken,
    ORGANIZATION,
    safeFetch,
    SERVER
} from './common.mjs';
import { err, info } from '../../lib/util.mjs';

// Delete Package
async function deletePackage(distribution, packageName, version, arch) {
    const url = `${SERVER}/api/packages/${ORGANIZATION}/debian/pool/${distribution}/${DEBIAN_COMPONENT}/${packageName}/${version}/${arch}`;
    await safeFetch(url, {
        method: 'DELETE',
        headers: {
            Authorization: `token ${getToken()}`
        }
    });
}

// Upload A Package
function getDebianPackageField(file, field) {
    const command = ['dpkg-deb', '--field', file, field];
    try {
        return child_process.execFileSync(command[0], command.slice(1), {
            stdio: ['ignore', 'pipe', 'inherit'],
            encoding: 'utf8'
        }).trim();
    } catch (e) {
        err('Unable To Parse Debian Package');
    }
}
async function uploadPackage(file, distribution) {
    // Parse Package
    const packageName = getDebianPackageField(file, 'Package');
    const version = getDebianPackageField(file, 'Version');
    const architecture = getDebianPackageField(file, 'Architecture');

    // Delete Old Package (If It Exists)
    await deletePackage(distribution, packageName, version, architecture);

    // Read File
    const data = fs.readFileSync(file);
    const buffer = new Uint8Array(data).buffer;

    // Upload
    const url = `${SERVER}/api/packages/${ORGANIZATION}/debian/pool/${distribution}/${DEBIAN_COMPONENT}/upload`;
    const response = await safeFetch(url, {
        method: 'PUT',
        headers: {
            Authorization: `token ${getToken()}`,
            'Content-Type': 'application/octet-stream'
        },
        body: buffer
    });
    if (!response.ok) {
        err('Unable To Upload Package');
    }
}

// Upload All Packages
export async function uploadPackages(distribution) {
    info('Uploading Packages To: ' + distribution);
    const out = getOutputDir();
    const files = fs.readdirSync(out);
    for (const file of files) {
        if (file.endsWith(DEBIAN_EXTENSION)) {
            const fullPath = path.join(out, file);
            info('Uploading Package: ' + fullPath);
            await uploadPackage(fullPath, distribution);
        }
    }
}