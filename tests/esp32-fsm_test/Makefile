#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := esp32-fsm-test

PORT_PATH = $(PWD)/../../esp32/include
#BUILD_DIR = $(PWD)/../../build/debug
BUILD_DIR = $(PWD)/../../build/release

EXTRA_CFLAGS = -I$(BUILD_DIR) -I$(PWD)/../../lwiot-core/include -I$(PORT_PATH)
EXTRA_CXXFLAGS = $(EXTRA_CFLAGS) -std=gnu++14

IDF_PATH := $(ESP32_IDF_PATH)
export IDF_PATH

include $(IDF_PATH)/make/project.mk

