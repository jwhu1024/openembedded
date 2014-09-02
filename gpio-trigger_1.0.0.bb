inherit linux-kernel-base module
DESCRIPTION = "Recipe for gpio-trigger"
HOMEPAGE = "http://www.bandrich.com/"
SECTION = "kernel/modules"
PRIORITY = "optional"
LICENSE = "BandRich Proprietary license"
LIC_FILES_CHKSUM = "file://COPYING;md5=7fee3e6baab22bd090666b7895972122"
PR = "r7"

KERNEL_VERSION = "${@get_kernelversion('${STAGING_KERNEL_DIR}')}"

SRC_URI = "				\
	file://debug.h 			\
	file://gpio-trigger.c		\
	file://gpio-trigger.h		\
	file://gpio-trigger.init	\
	file://gpio-trigger-test.c	\
	file://Makefile			\
	file://COPYING			\
"

S = "${WORKDIR}"
KMODULE_NAME = "gpio-trigger"

INITSCRIPT_NAME		= "gpio-trigger"
INITSCRIPT_PARAM	= "start 81 2 3 4 5 . stop 41 0 1 6 ."

inherit update-rc.d

do_compile () {
	${CC} ${CFLAGS} ${LDFLAGS} -o gpio-trigger-test gpio-trigger-test.c -lpthread

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

	install -d ${D}${bindir}
	install -m 0755 ${S}/gpio-trigger-test ${D}${bindir}

	install -d ${D}${sysconfdir}/init.d/
	install -m 755 ${S}/gpio-trigger.init ${D}${sysconfdir}/init.d/gpio-trigger
}

pkg_postinst () {
	[ -n "$D" ] && OPT="-r $D" || OPT="-s"
	update-rc.d $OPT -f ${INITSCRIPT_NAME} remove
	update-rc.d $OPT ${INITSCRIPT_NAME} ${INITSCRIPT_PARAMS}
}

FILES_${PN} += "\
	${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${KMODULE_NAME}/${KMODULE_NAME}.ko"

FILES_${PN} += "${bindir}"
FILES_${PN} += "${sysconfdir}"