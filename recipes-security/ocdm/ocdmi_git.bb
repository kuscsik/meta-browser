#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "Open Content Decryption Module"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=ea83f8bc099c40bde8c4f2441a6eb40b"

SRC_URI = "git://github.com/kuscsik/linaro-cdmi.git;protocol=https;branch=master"
SRCREV_pn-ocdmi ?= "${AUTOREV}"

S = "${WORKDIR}/git"

# * use-playready : Enables support for Playready CDMI.
#
# * debug-build : Builds OCDM with debug symbols and verbose logging.

PACKAGECONFIG ?= "use-playready debug-build"

DEPENDS_append = "  openssl "

PACKAGECONFIG[use-playready] = "--enable-playready"
PACKAGECONFIG[debug-build] = "--enable-debug"

# Only ClearKey implementation depends on ssl
DEPENDS_remove = " \
  ${@base_contains('PACKAGECONFIG','use-playready','openssl','',d)} \
  "
DEPENDS_append = " \
  ${@base_contains('PACKAGECONFIG','use-playready','optee-playready playready','',d)} \
 "

pkg_postinst_${PN} () {
    echo 127.0.0.1    ${MACHINE} >> $D/etc/hosts
}

inherit autotools
