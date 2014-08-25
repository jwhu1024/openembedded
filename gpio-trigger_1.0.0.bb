inherit linux-kernel-base module
DESCRIPTION = "Recipe for gpio-trigger"
HOMEPAGE = "http://www.bandrich.com/"
SECTION = "kernel/modules"
PRIORITY = "optional"
LICENSE = "BandRich Proprietary license"
LIC_FILES_CHKSUM = "file://COPYING;md5=7fee3e6baab22bd090666b7895972122"
PR = "r1"

KERNEL_VERSION = "${@get_kernelversion('${STAGING_KERNEL_DIR}')}"

SRC_URI = "					\
	file://gpio-trigger.c	\
	file://gpio-trigger.h	\
	file://Makefile			\
	file://COPYING			\
"

S = "${WORKDIR}"
KMODULE_NAME = "gpio-trigger"

do_compile () {
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS CC LD CPP
	oe_runmake 'MODPATH="${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/ecu"' \
		'KERNEL_SOURCE="${STAGING_KERNEL_DIR}"' \
		'KDIR="${STAGING_KERNEL_DIR}"' \
		'KERNEL_VERSION="${KERNEL_VERSION}"' \
		'CC="${KERNEL_CC}"' \
		'LD="${KERNEL_LD}"'
}

do_install () {
	install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${KMODULE_NAME}
	install -m 0644 ${S}/${KMODULE_NAME}*${KERNEL_OBJECT_SUFFIX} ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${KMODULE_NAME}
}

FILES_${PN} += "\
	${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${KMODULE_NAME}/${KMODULE_NAME}.ko"
