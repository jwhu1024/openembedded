DESCRIPTION = "QMI command line interface to read/write NV item"
LICENSE = "BandRich-Proprietary"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=b094a1f3afde551ede1eee992f3ed7e9"

DEPENDS += "qmi-framework"
RDEPENDS += "qmi-framework"

PR = "r001"

SRC_URI = "file://build				\
		   file://src/*				\
		   file://inc/*				\
		   file://LICENSE			\
		   file://CMakeLists.txt	\
		  "

S = "${WORKDIR}/build"

inherit cmake

do_configure () {
	cmake .. -DINC_DIR:PATH=${WORKSPACE}		\
			 -DLIB_DIR:PATH=${STAGING_LIBDIR}	\
			 -DCMAKE_BUILD_TYPE=Release
}

do_compile() {
	make VERBOSE=1
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/bin/qmi-cli ${D}${bindir}
}

FILE_${PN} = "${bindir}"