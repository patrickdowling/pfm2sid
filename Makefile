TOOLCHAIN_PATH ?= ~/dev/toolchain/arm-gnu-toolchain-12.3.rel1-darwin-x86_64-arm-none-eabi/

###
## Project config
#
PROJECT := pfm2sid
PROJECT_INCLUDE_DIRS = ./src
PROJECT_SRC_DIRS = ./src ./src/drivers ./src/ui ./src/menu ./src/midi ./src/synth ./src/sidbits
PROJECT_RESOURCE_DIR = ./resources
PROJECT_RESOURCE_SCRIPT = $(PROJECT_RESOURCE_DIR)/resources.py
STM32X_CPPSTD := c++17
OPTIMIZE = -O3

PROJECT_DEFINES += PFM2SID_DEBUG_ENABLE
#PROJECT_DEFINES += USE_FULL_ASSERT

# Enable float printf, this has side effects like requiring flash, double promotion, etc.
PROJECT_LINKER_FLAGS += -u _printf_float
#DISABLE_WDOUBLE_PROMOTION := TRUE

OPTIONAL_C_FLAGS += -Wno-psabi

# PreenF405
RAM_SIZE     = 128K
CCM_RAM_SIZE = 64K
MIN_STACK_SIZE = 2K
#FLASH_SETTINGS_SIZE = 1K
ENABLE_LIBC_INIT_ARRAY = TRUE
ENABLE_CCM_STACK = TRUE

ENABLE_BOOTLOADER ?= FALSE
ifeq "TRUE" "$(ENABLE_BOOTLOADER)"
# The preenfm2 bootloader is huge!
FLASH_ORIGIN = 0x08040000
FLASH_SIZE = 768K
else
FLASH_ORIGIN = 0x08000000
FLASH_SIZE   = 1024K
endif

MAX_FRAME_SIZE = 192 # reSID

MODEL = STM32F40_41xxx
F_CPU = 168000000UL
HSE_VALUE = 12000000UL

PROJECT_IOC_FILE = ./stm32/preenF405.ioc
PINOUT_OPTIONS = --typedef --clocks=manual

# reSID as external resource
RESID_SRC_DIR := ./extern/reSID/src
PROJECT_INCLUDE_DIRS += $(RESID_SRC_DIR)
PROJECT_SRC_DIRS += $(RESID_SRC_DIR)

PROJECT_DEFINES += RESID_FILTER_CONSTEXPR
PROJECT_DEFINES += VERSION=\"0.16\"
PROJECT_DEFINES += RESID_RAW_OUTPUT
#PROJECT_DEFINES += RESID_ENABLE_INTERPOLATE

PROJECT_DEFINES += MIDI_PARSER_RX_BUFFER_SIZE=256

# Binary blobs (this is somewhat temporary)
BINFILES = $(notdir $(wildcard $(PROJECT_RESOURCE_DIR)/*.dmp))
EXTRA_OBJS += $(patsubst %,$(OBJDIR)%,$(BINFILES:.dmp=.o))

include stm32x/makefile

# This step is somewhat convoluted to avoid endless variable names since the whole input path is used.
# Also we want them in a readonly section, not RAM
PWD := $(shell pwd)
$(BUILD_DIR)%.o: %.dmp
	$(ECHO) BIN $<
	$(Q)cd $(dir $<) && $(LD) -r -b binary -o $(PWD)/$@.ld $(notdir $<) && $(OBJCOPY) -v --rename-section .data=.rodata $(PWD)/$@.ld $(PWD)/$@

# clang-format
CLANG_FORMAT_OPTS = -i --style=file
ifdef VERBOSE
	CLANG_FORMAT_OPTS += --verbose
endif
CLANG_FORMAT_DIRS = ./src
CLANG_FORMAT_SHALLOW_DIRS = ./test
.PHONY: format
format:
	$(Q)clang-format $(CLANG_FORMAT_OPTS) \
		$(foreach dir,$(CLANG_FORMAT_DIRS),$(shell find $(dir) -name \*.cc -o -name \*.h)) \
		$(foreach dir,$(CLANG_FORMAT_SHALLOW_DIRS),$(shell find $(dir) -depth 1 -name \*.cc -o -depth 1 -name \*.h))
