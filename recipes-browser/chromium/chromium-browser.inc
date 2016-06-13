include chromium.inc
inherit gclient-fetch

DESCRIPTION = "Chromium browser"
DEPENDS += "libgnome-keyring"

SRC_URI = "\
        gclient://chromium.googlesource.com/chromium/src.git;protocol=https;tag=51.0.2704.90 \
        https://commondatastorage.googleapis.com/chromium-browser-clang/Linux_x64/clang-263324-1.tgz;subdir=gclient/src/third_party/llvm-build/Release+Asserts/  \
"

SRC_URI[md5sum] = "b8fc4d0cadcc965fc43bf3fee01ca32e"
SRC_URI[sha256sum] = "c4f12b94351708a9d0756c85a15f123426ef47c5920c76da73b2fe7de3336de9"

S = "${WORKDIR}/gclient/src"  

PACKAGECONFIG ??= "use-egl"

# this makes sure the dependencies for the EGL mode are present; otherwise, the configure scripts
# automatically and silently fall back to GLX
PACKAGECONFIG[use-egl] = ",,virtual/egl virtual/libgles2"

# Empty PACKAGECONFIG options listed here to avoid warnings.
# The .bb file should use these to conditionally add patches
# and command-line switches (extra dependencies should not
# be necessary but are OK to add).
PACKAGECONFIG[disable-api-keys-info-bar] = ""
PACKAGECONFIG[component-build] = ""
PACKAGECONFIG[ignore-lost-context] = ""
PACKAGECONFIG[impl-side-painting] = ""
PACKAGECONFIG[use-egl] = ""
PACKAGECONFIG[kiosk-mode] = ""
PACKAGECONFIG[proprietary-codecs] = ""


# Conditionally add ozone-wayland and its patches to the Chromium sources
# You can override this by setting CHROMIUM_ENABLE_WAYLAND=1 or CHROMIUM_ENABLE_WAYLAND=0 in local.conf
CHROMIUM_ENABLE_WAYLAND ??= "${@bb.utils.contains('DISTRO_FEATURES', 'x11', '0', \
                     bb.utils.contains('DISTRO_FEATURES', 'wayland', '1', \
                     '0', d),d)}"

# ozone-wayland will be cloned into a directory with this name
OZONE_WAYLAND_GIT_DESTSUFFIX ?= "ozone-wayland-git"
# Use glob to filter out the patches that shall be applied against ozone-wayland.
# Useful if some of the patches would break the build.
OZONE_WAYLAND_PATCH_FILE_GLOB ?= "*.patch"

# Variable for extra ozone-wayland patches, typically extended by BSP layer .bbappends
# IMPORTANT: do not simply add extra ozone-wayland patches to the SRC_URI in a
# .bbappend, since the base ozone-wayland patches need to be applied first (see below)
OZONE_WAYLAND_EXTRA_PATCHES ?= ""


# These are present as their own variables, since they have changed between versions
# a few times in the past already; making them variables makes it easier to handle that
CHROMIUM_X11_DEPENDS ?= ""
CHROMIUM_X11_GYP_DEFINES ?= ""
CHROMIUM_WAYLAND_DEPENDS ?= ""
CHROMIUM_WAYLAND_GYP_DEFINES ?= ""


CHROMIUM_EXTRA_ARGS ?= " \
        ${@bb.utils.contains('PACKAGECONFIG', 'use-egl', '--use-gl=egl', '', d)} \
        ${@bb.utils.contains('PACKAGECONFIG', 'ignore-lost-context', '--gpu-no-context-lost', '', d)} \
        ${@bb.utils.contains('PACKAGECONFIG', 'impl-side-painting', '--enable-gpu-rasterization --enable-impl-side-painting', '', d)} \
        ${@bb.utils.contains('PACKAGECONFIG', 'kiosk-mode', '--start-fullscreen --kiosk --no-first-run', '', d)} \
"



# Some sanity checks.
python do_check_variables() {
    CHROMIUM_BUILD_TYPE = d.getVar('CHROMIUM_BUILD_TYPE', True)
    CHROMIUM_ENABLE_WAYLAND = d.getVar('CHROMIUM_ENABLE_WAYLAND', True)
    DISTRO_FEATURES = d.getVar("DISTRO_FEATURES", True).split()
    if CHROMIUM_BUILD_TYPE not in ['Release', 'Debug']:
        bb.fatal("Wrong value for CHROMIUM_BUILD_TYPE. Please set it either to \'Release\' or to \'Debug\'")
    if CHROMIUM_ENABLE_WAYLAND not in ['0', '1']:
        bb.fatal("Wrong value for CHROMIUM_ENABLE_WAYLAND. Please set it to \'1\' to enable the feature or to \'0\' to disable it")
    if ( (CHROMIUM_ENABLE_WAYLAND == '1') and ('wayland' not in DISTRO_FEATURES) ):
        bb.warn("You have selected to build Chromium with Wayland support, but you have not enabled wayland in DISTRO_FEATURES")
    if ( (CHROMIUM_ENABLE_WAYLAND != '1') and ('x11' not in DISTRO_FEATURES) ):
        bb.warn("You have selected to build Chromium without Wayland support, but you have not enabled x11 in DISTRO_FEATURES")
    # Print both on log.do_checkvariables and on the console the configuration that is selected.
    # This useful both for throubleshooting and for checking how the build is finally configured.
    if (CHROMIUM_ENABLE_WAYLAND == '1'):
        bb.plain("INFO: Chromium has been configured with Wayland support (ozone-wayland). Build type is \'%s\'" %CHROMIUM_BUILD_TYPE)
    else:
        bb.plain("INFO: Chromium has been configured without Wayland support. Build type is \'%s\'" %CHROMIUM_BUILD_TYPE)
}
addtask check_variables before do_fetch

EXTRA_OEGYP = " \
	-Dangle_use_commit_id=0 \
	-Ddisable_fatal_linker_warnings=1 \
	-Dlinux_use_bundled_gold=0 \
	-Dlinux_use_bundled_binutils=0 \
	${@bb.utils.contains('DISTRO_FEATURES', 'ld-is-gold', '-Dlinux_use_gold_flags=1', '-Dlinux_use_gold_flags=0', d)} \
	-I ${WORKDIR}/oe-defaults.gypi \
	-I ${WORKDIR}/include.gypi \
	${@bb.utils.contains('PACKAGECONFIG', 'component-build', '-I ${WORKDIR}/component-build.gypi', '', d)} \
	${@bb.utils.contains('PACKAGECONFIG', 'proprietary-codecs', '-Dproprietary_codecs=1 -Dffmpeg_branding=Chrome', '', d)} \
	-f ninja \
"

python() {
    if d.getVar('CHROMIUM_ENABLE_WAYLAND', True) == '1':
        d.appendVar('DEPENDS', ' %s ' % d.getVar('CHROMIUM_WAYLAND_DEPENDS', True))
        d.appendVar('GYP_DEFINES', ' %s ' % d.getVar('CHROMIUM_WAYLAND_GYP_DEFINES', True))
    else:
        d.appendVar('DEPENDS', ' %s ' % d.getVar('CHROMIUM_X11_DEPENDS', True))
        d.appendVar('GYP_DEFINES', ' %s ' % d.getVar('CHROMIUM_X11_GYP_DEFINES', True))
}

do_configure_append() {
	build/gyp_chromium --depth=. ${EXTRA_OEGYP}
  python build/util/lastchange.py -o build/util/LASTCHANGE
  python build/util/lastchange.py --git-hash-only \
    -s third_party/WebKit \
    -o build/util/LASTCHANGE.blink
}

do_unpack_append() {
}

do_compile() {
        # build with ninja
        ninja -C ${S}/out/${CHROMIUM_BUILD_TYPE} ${PARALLEL_MAKE} chrome chrome_sandbox
}

do_install_append() {

	# Add extra command line arguments to google-chrome script by modifying
        # the dummy "CHROME_EXTRA_ARGS" line
        sed -i "s/^CHROME_EXTRA_ARGS=\"\"/CHROME_EXTRA_ARGS=\"${CHROMIUM_EXTRA_ARGS}\"/" ${D}${bindir}/google-chrome

        # update ROOT_HOME with the root user's $HOME
        sed -i "s#ROOT_HOME#${ROOT_HOME}#" ${D}${bindir}/google-chrome

	# Always adding this libdir (not just with component builds), because the
        # LD_LIBRARY_PATH line in the google-chromium script refers to it
        install -d ${D}${libdir}/${BPN}/
        if [ -n "${@bb.utils.contains('PACKAGECONFIG', 'component-build', 'component-build', '', d)}" ]; then
                install -m 0755 ${B}/out/${CHROMIUM_BUILD_TYPE}/lib/*.so ${D}${libdir}/${BPN}/
        fi
}
