SPIFFSGEN_FLASH_IN_PROJECT=

ifdef CONFIG_SPIFFS_USE_MAGIC
USE_MAGIC = "--use-magic"
else
USE_MAGIC = ""
endif

ifdef CONFIG_SPIFFS_USE_MAGIC_LENGTH
USE_MAGIC_LEN = "--use-magic-len" 
else
USE_MAGIC_LEN = ""
endif

# spiffs_create_image
#
# Create a spiffs image of the specified directory on the host during build and optionally
# have the created image flashed using `make flash`
define spiffs_create_image


$(1)_bin: check_python_dependencies
	$(PYTHON) $(SPIFFSGEN_PY) $(3) $(2) $(BUILD_DIR_BASE)/$(1).bin \
	--page-size=$(CONFIG_SPIFFS_PAGE_SIZE) \
	--obj-name-len=$(CONFIG_SPIFFS_OBJ_NAME_LEN) \
	--meta-len=$(CONFIG_SPIFFS_META_LENGTH) \
	$(USE_MAGIC) \
	$(USE_MAGIC_LEN) 

all_binaries: $(1)_bin
print_flash_cmd: $(1)_bin

SPIFFS_IMAGE = $(BUILD_DIR_BASE)/$(1).bin
SPIFFS_OFFSET = $(4)

# Append the created binary to esptool_py args if FLASH_IN_PROJECT is set
ifeq ($(5), FLASH_IN_PROJECT)
SPIFFSGEN_FLASH_IN_PROJECT += $(1)
endif
endef

ESPTOOL_ALL_FLASH_ARGS += $(SPIFFS_OFFSET) $(SPIFFS_IMAGE)

