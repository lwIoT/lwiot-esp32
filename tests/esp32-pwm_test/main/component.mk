#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

BUILD_DIR = $(PWD)/../../build/debug
COMPONENT_ADD_LDFLAGS += -L$(BUILD_DIR)/lwiot-core/source -L$(BUILD_DIR)/esp32 -llwiot -llwiot-platform
