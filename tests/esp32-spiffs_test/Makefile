
#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := esp32_spifs-test

PORT_PATH = $(PWD)/../../esp32/include
BUILD_DIR = $(PWD)/../../build/debug

EXTRA_CFLAGS = -I$(BUILD_DIR) -I$(PWD)/../../lwiot-core/include -I$(PORT_PATH)
EXTRA_CXXFLAGS = $(EXTRA_CFLAGS) -std=gnu++14

SPIFFSGEN_PY := ../../lwiot-core/scripts/spiffsgen.py
IDF_PATH := $(ESP32_IDF_PATH)

export IDF_PATH
export SPIFFSGEN_PY

include $(IDF_PATH)/make/project.mk
include $(PWD)/../../cmake/spiffs_tools.mk

# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'make flash'. 

$(eval $(call spiffs_create_image,storage,spiffs_image,0x80000,0x310000,FLASH_IN_PROJECT))
