#!/usr/bin/env node

// Arguments
if (process.argv.length < 4) {
    throw new Error('Invalid Arguments');
}
const mode = process.argv[2];
const arch = process.argv[3];

// Data
const id = `com.thebrokenrail.MCPIReborn${mode === 'server' ? 'Server' : ''}`;
const name = `minecraft-pi-reborn-${mode}`;
const updateURL = `https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/${name}-latest-${arch}.AppImage.zsync`;

// APT Data
const apt_distribution = 'bullseye';
const apt_key_url = 'https://ftp-master.debian.org/keys/archive-key-11.asc';

// Version
const fs = require('fs');
const version = fs.readFileSync('VERSION', 'utf8').trim();

// Packages/Dependencies
const packages = [
    'libc6',
    'libstdc++6',
    'patchelf'
];
if (mode === 'client') {
    // GLFW's Dependencies Aren't Included As They Should Be Provided By The Host System
    packages.push(
        'libgtk-3-0',
        'libglib2.0-0',
        'libgdk-pixbuf2.0-0',
        'shared-mime-info',
        'libfreeimage3',
        'libopenal1'
    );
}
if (arch !== 'armhf') {
    packages.push(
        'libc6-armhf-cross',
        'libstdc++6-armhf-cross'
    );
    if (arch !== 'arm64') {
        packages.push('qemu-user');
    }
}

// Package Exclusions
const packageExclusions = [
    // Exclude Unneeded Packages
    'humanity-icon-theme',
    'adwaita-icon-theme',
    'libxml2',
    '*systemd*',
    'dconf-service',
    'dconf-gsettings-backend',
    'librest-*',
    'libcups2',
    'libcolord2',
    'libmount1'
];

// APT
const apt = {
    arch: arch,
    sources: [
        {
            sourceline: `deb [arch=${arch}] http://deb.debian.org/debian/ ${apt_distribution} main`,
            key_url: apt_key_url
        },
        {
            sourceline: `deb [arch=${arch}] http://deb.debian.org/debian/ ${apt_distribution}-updates main`,
            key_url: apt_key_url
        }
    ],
    include: packages,
    exclude: packageExclusions
};

// Get Architecture Triplet
const triplet = {
    'amd64': 'x86_64-linux-gnu',
    'i386': 'i386-linux-gnu',
    'arm64': 'aarch64-linux-gnu',
    'armhf': 'arm-linux-gnueabihf'
}[arch];
if (!triplet) {
    throw new Error();
}

// Files
const files = {
    exclude: [
        // Exclude Unused Files
        `usr/lib/${triplet}/gconv`,
        'usr/share/man',
        'usr/share/doc/*/README.*',
        'usr/share/doc/*/changelog.*',
        'usr/share/doc/*/NEWS.*',
        'usr/share/doc/*/TODO.*',
        'usr/include',
        'usr/share/locale',
        'usr/share/help',
        'usr/bin/update-mime-database'
    ]
};

// Script After Bundling Dependencies
const afterBundle = [
    // Remove Unused QEMU Binaries
    'find ./AppDir/usr/bin -maxdepth 1 -name \'qemu-*\' -a ! -name \'qemu-arm\' -delete'
];

// Runtime
const runtime = {
    env: mode === 'client' ? {
        // Make GTK Work (Zenity Uses GTK)
        GTK_EXE_PREFIX: '${APPDIR}/usr',
        GTK_PATH: `\${APPDIR}/usr/lib/${triplet}/gtk-3.0`,
        GTK_DATA_PREFIX: '${APPDIR}',
        GTK_THEME: 'Default',
        XDG_DATA_DIRS: '${APPDIR}/share:${APPDIR}/usr/share',
        APPDIR_LIBRARY_PATH: `\${APPDIR}/usr/lib/${triplet}:\${APPDIR}/usr/${triplet}/lib:\${APPDIR}/lib/${triplet}:\${APPDIR}/usr/lib:\${APPDIR}/usr/lib/${triplet}/gdk-pixbuf-2.0/2.10.0/loaders`
    } : undefined,
    preserve: arch !== 'armhf' ? [
        // On non-ARM32 systems, an ARM32 linker is embedded, this
        // prevents AppImage-Builder from modifying ARM32 binaries
        // to use a (usually non-existent) system linker.
        `usr/lib/${name}/minecraft-pi`,
        `usr/lib/${name}/**/*.so`,
        'usr/arm-linux-gnueabihf/lib'
    ] : undefined,
    // libapprun_hooks.so Is Buggy And Unneeded
    no_hooks: true
};

// AppDir
const appDir = {
    path: `./AppDir`,
    app_info: {
        id: id,
        name: `${name}`,
        icon: id,
        version: version,
        exec: `usr/bin/${name}`,
        exec_args: '$@'
    },
    apt: apt,
    files: files,
    after_bundle: afterBundle,
    runtime: runtime
};

// Build Script
const script = [
    `rm -rf ./build/${mode}-${arch}`,
    `./scripts/setup.sh ${mode} ${arch} -DMCPI_IS_APPIMAGE_BUILD=ON`,
    `rm -rf ./out/${mode}-${arch}`,
    `./scripts/build.sh ${mode} ${arch}`,
    'rm -rf ./AppDir',
    `cp -ar ./out/${mode}-${arch} ./AppDir`
];

// AppImage
const appImageArch = {
    'amd64': 'x86_64',
    'i386': 'i686',
    'arm64': 'aarch64',
    'armhf': 'armhf'
}[arch];
if (!appImageArch) {
    throw new Error();
}
const appImage = {
    arch: appImageArch,
    file_name: `./out/${name}-${version}-${arch}.AppImage`,
    'update-information': `zsync|${updateURL}`
};

// Root
const root = {
    version: 1,
    AppDir: appDir,
    script: script,
    AppImage: appImage
};

// Write
fs.writeFileSync(`AppImageBuilder.yml`, JSON.stringify(root, null, 4));
