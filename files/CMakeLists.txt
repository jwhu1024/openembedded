cmake_minimum_required (VERSION 2.6)

# setting project name
project(qmi-cli)

# compiler option for debug mode (binary about 315K)
SET(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb -D__DEBUG__")

# compiler option for release mode (binary about 7.9K)
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

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

# compiler flags
add_definitions(-Wall -Wundef
		-Wstrict-prototypes
		-Wno-trigraphs
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
