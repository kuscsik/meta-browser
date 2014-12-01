DESCRIPTION = "Chromium Embedded Framework"

LICENSE = "BSD"

# Based on Chromium dependencies
DEPENDS = " xz-native pciutils pulseaudio xextproto pango cairo nss zlib-native cups ninja-native gconf libexif libxkbcommon fontconfig libdrm virtual/egl virtual/libgles2"

RDEPENDS_${PN} = " pango cairo fontconfig pciutils pulseaudio freetype ttf-ubuntu-mono fontconfig-utils"

COMPATIBLE_MACHINE = "(-)"
COMPATIBLE_MACHINE_i586 = "(.*)"
COMPATIBLE_MACHINE_x86-64 = "(.*)"
COMPATIBLE_MACHINE_armv6 = "(.*)"
COMPATIBLE_MACHINE_armv7a = "(.*)"

SRC_URI[depot.md5sum] = "58b80167bce721bb7ef18cdfbe1a9b1d"
SRC_URI[depot.sha256sum] = "8a6a95929ef06d878fb11909807b80a5a2fed9c38f6d274333bdccb0f361f9fd"

SRC_URI = "\
	http://people.linaro.org/~zoltan.kuscsik/chromium-browser/chromium_rev_${PV}.tar.xz\
	git://github.com/kuscsik/chromiumembedded.git;protocol=https;destsuffix=src/cef;branch=aura\
	git://github.com/kuscsik/ozone-egl.git;protocol=https;destsuffix=src/ui/ozone/platform/egl;branch=master\
	git://chromium.googlesource.com/chromium/tools/depot_tools.git;protocol=https;destsuffix=depot_tools;branch=master\
        file://01_get_svn_version_from_LASTCHANGE.patch\
"
S = "${WORKDIR}/chromium_rev_${PV}"

SRC_URI[md5sum] = "9efbb50283b731042e62b9bd5e312b2f"
SRC_URI[sha256sum] = "f608e97dadf6ea4d885b24fd876896d46840fa39bf743ea2025075aee9fb348d"

SRCREV = "${AUTOREV}"

export CHROMIUM_BUILD_TYPE="Release"

GYP_ARCH_DEFINES_armv7a = " target_arch=arm arm_float_abi=hard"
GYP_ARCH_DEFINES_i586 = " target_arch=ia32"

# Recommended generator for cef3

export GYP_GENERATORS="ninja"

export BUILD_TARGET_ARCH="${TARGET_ARCH}"

export GYP_DEFINES="${GYP_ARCH_DEFINES} release_extra_cflags='-Wno-error=unused-local-typedefs' sysroot=''"

inherit gettext

do_configure() {
    # LD workaround taken from meta-browser
    # replace LD with CXX, to workaround a possible gyp issue?
    export PATH=${WORKDIR}/depot_tools:"$PATH"
    LD="${CXX}" export LD
    CC="${CC}" export CC
    CXX="${CXX}" export CXX
    CC_host="gcc" export CC_host
    CXX_host="g++" export CXX_host
    # End of LD Workaround

    #-----------------------
    # Configure and build cef
    #------------------------
    (cd cef/;  ./cef_create_projects.sh -I ${BUILD_TARGET_ARCH}_ozone.gypi --depth ../)
}

# Workaround to disable qa_configure
do_qa_configure() {
    echo "do_qa_configure"
}

do_compile() {
    ninja -C out/${CHROMIUM_BUILD_TYPE} cefsimple
}

do_install() {
   install -d ${D}${bindir}
   install -d ${D}${libdir}
   install -d ${D}${bindir}/chrome
   install -m 0755 ${S}/out/${CHROMIUM_BUILD_TYPE}/cefsimple ${D}${bindir}/chrome/cefsimple
   install -m 0644 ${S}/out/${CHROMIUM_BUILD_TYPE}/lib/libcef.so ${D}${libdir}/
   install -m 0644 ${S}/out/${CHROMIUM_BUILD_TYPE}/icudtl.dat ${D}${bindir}/chrome/
   install -m 0755 ${S}/out/${CHROMIUM_BUILD_TYPE}/libosmesa.so ${D}${bindir}/chrome/
   install -m 0755 ${S}/out/${CHROMIUM_BUILD_TYPE}/libffmpegsumo.so ${D}${bindir}/chrome/
   install -m 0755 ${S}/out/${CHROMIUM_BUILD_TYPE}/cef_100_percent.pak ${D}${bindir}/chrome/
   install -m 0755 ${S}/out/${CHROMIUM_BUILD_TYPE}/cef_200_percent.pak ${D}${bindir}/chrome/
   install -m 0755 ${S}/out/${CHROMIUM_BUILD_TYPE}/cef_resources.pak ${D}${bindir}/chrome/
   install -m 0755 ${S}/out/${CHROMIUM_BUILD_TYPE}/cef.pak ${D}${bindir}/chrome/

   install -d ${D}${bindir}/chrome/locales/
   install -m 0644 ${S}/out/${CHROMIUM_BUILD_TYPE}/locales/en-US.pak ${D}${bindir}/chrome/locales
}

INSANE_SKIP_${PN} = "dev-so ldflags textrel"
FILES_SOLIBSDEV = ""

FILES_${PN} = "${bindir}/chrome/ ${libdir}/"
#install cefsimple
FILES_${PN} += "${bindir}/chrome/cefsimple"
#install resources
FILES_${PN} += "${bindir}/chrome/*.pak"
#install locales
FILES_${PN} += "${bindir}/chrome/locales/*.pak"

FILES_${PN}-dbg += "${bindir}/chrome/.debug/ ${libdir}/.debug/"
PACKAGE_DEBUG_SPLIT_STYLE = "debug-without-src"
