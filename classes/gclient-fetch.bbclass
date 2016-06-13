#
# gclient-fetch class
#
# Registers GCLIENT method for Bitbake fetch2.
#

GCLIENT ?= "gclient"
GCLIENT_ARCHFLAGS ?= "--arch=${TARGET_ARCH} --target_arch=${TARGET_ARCH}"

python () {
        import gclient
        bb.fetch2.methods.append( gclient.Gclient() )
}
