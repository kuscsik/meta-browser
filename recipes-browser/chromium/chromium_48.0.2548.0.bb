include chromium-browser.inc

SRC_URI += "\
"

LIC_FILES_CHKSUM = "file://LICENSE;md5=0fca02217a5d49a14dfe2d11837bb34d"

EXTRA_OEGYP += " \
	-Dv8_use_external_startup_data=0 \
"

OZONE_WAYLAND_GIT_BRANCH = "Milestone-SouthSister"
OZONE_WAYLAND_GIT_SRCREV = "c605505044af3345a276abbd7c29fd53db1dea40"

OZONE_WAYLAND_EXTRA_PATCHES = " \
	file://chromium-48/0006-Remove-GBM-support-from-wayland.gyp.patch \
	file://chromium-48/0007-Workaround-for-glib-related-build-error-with-ozone-w.patch \
"

# Component build is unsupported in ozone-wayland for Chromium 48
python() {
    if (d.getVar('CHROMIUM_ENABLE_WAYLAND', True) == '1'):
        if bb.utils.contains('PACKAGECONFIG', 'component-build', True, False, d):
            bb.fatal("Chromium 48 Wayland version cannot be built in component-mode")
}

CHROMIUM_X11_DEPENDS = "xextproto gtk+ libxi libxss"
CHROMIUM_X11_GYP_DEFINES = ""
CHROMIUM_WAYLAND_DEPENDS = "wayland libxkbcommon"
CHROMIUM_WAYLAND_GYP_DEFINES = "use_ash=1 use_aura=1 chromeos=0 use_ozone=1 use_xkbcommon=1"
