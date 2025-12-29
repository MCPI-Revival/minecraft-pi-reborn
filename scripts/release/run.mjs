#!/usr/bin/env node
import { info, err } from '../lib/util.mjs';
import {
    DEBIAN_STABLE_DISTRIBUTION,
    DEBIAN_UNSTABLE_DISTRIBUTION,
    getToken,
    GIT_TAG_PREFIX,
    safeFetch,
    SERVER,
    SLUG,
    VERSION
} from './lib/common.mjs';
import { uploadPackages } from './lib/upload-packages.mjs';
import { createRelease } from './lib/create-release.mjs';

// Get Git Reference
let ref = process.env.REF;
const dryRun = !ref;
if (dryRun) {
    // Enable Dry-Run Mode
    info('Running In Dry-Run Mode');
    safeFetch.dryRun = true;
    getToken.ignoreMissing = true;
    ref = GIT_TAG_PREFIX + VERSION;
}

// Logging
info(`Target: ${SERVER}/${SLUG}`);
info(`Version: ${VERSION}`);

// Select Mode
(async function () {
    if (ref.startsWith(GIT_TAG_PREFIX)) {
        // Building Tag, Do A Full Release
        const tag = ref.substring(GIT_TAG_PREFIX.length);
        if (tag !== VERSION) {
            err('Tag/Version Mismatch');
        }
        await uploadPackages(DEBIAN_STABLE_DISTRIBUTION);
        await createRelease(dryRun, tag);
    } else {
        // Standard Commit, Only Upload To Unstable Repository
        await uploadPackages(DEBIAN_UNSTABLE_DISTRIBUTION);
        await createRelease(true, VERSION);
    }
})();