cmake_minimum_required (VERSION 2.6)

# setting project name
project(qmi-cli)

# setting header path
include_directories(
	${CMAKE_SOURCE_DIR}/inc
	${INC_DIR}/qmi-framework/inc
	${INC_DIR}/qmi-framework/qcci/src
	${INC_DIR}/qmi-framework/qcci/inc
)

link_directories(
	${LIB_DIR}
)

# own flag
add_definitions(-D__DEBUG__)

# compiler flags
add_definitions(-Wall -Wundef
		-Wstrict-prototypes
		-Wno-trigraphs
		-g -O0
		-fno-inline
		-fno-short-enums
		-fpic)

# output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./bin")

# generate binary
add_executable(qmi-cli
	./src/qmi-cli-main.c
	./src/qmi-cli-handler.c
	./src/qmi-cli-func.c
)

# setting library to link
target_link_libraries(qmi-cli libqmi_nv.so)
