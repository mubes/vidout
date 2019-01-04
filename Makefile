#VERBOSE=1
#DEBUG ?= 1
CROSS_COMPILE ?= $(HOME)/bin/armgcc/bin/arm-none-eabi-

##########################################################################
# User configuration and firmware specific object files
##########################################################################

PROJNAME=vidout

# Overall system defines for compilation
ifdef DEBUG
GCC_DEFINE=-DDEBUG
OPT_LEVEL = 
VARIANT = "Debug"
else
GCC_DEFINE= 
OPT_LEVEL = -O2
VARIANT = "Release"
endif

LDLIBS = 
GCC_DEFINE+= -std=gnu99 -DSTM32F103xB -DSTM32F10X_MD

# The target, flash and ram of the microprocessor.
TARGET = STM32F103C8
CORTEX_TYPE=m3
LD_SCRIPT = config/stm32f103c8.ld
CPU_TYPE = cortex-$(CORTEX_TYPE)

CFILES =
SFILES =
OLOC = ofiles
INCLUDE_PATHS = -I$(OLOC)
DEBUG_OPTS = -g3 -gdwarf-2 -ggdb

##########################################################################
# Generic multi-project files
##########################################################################

# CMSIS Files
# ===========
CMSIS_DIR=thirdparty/CMSIS
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(CMSIS_DIR)/inc -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(CMSIS_DIR)/src -name "*.c" -print)
SFILES += $(shell find $(CMSIS_DIR)/src -name "*.s" -print)

# Video Files
# ===========
VIDEO_DIR=vidout
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(VIDEO_DIR) -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(VIDEO_DIR) -name "*.c" -print)

##########################################################################
# Project-specific files
##########################################################################

# Main Files
# ==========
App_DIR=app
App_Inc_DIR=app
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(App_Inc_DIR) -name "*.h" -exec dirname {} \; | uniq ))

CFILES += $(shell find $(App_DIR) -name "*.c" -print)

##########################################################################
# GNU GCC compiler prefix and location
##########################################################################

STYLE = clang-format
AS = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
GDB = $(CROSS_COMPILE)gdb
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
MAKE = make
OUTFILE = firmware

##########################################################################
# Quietening
##########################################################################

ifdef VERBOSE
cmd = $1
Q :=
else
cmd = @$(if $(value 2),echo "$2";)$1
Q := @
endif

ifdef DEBUG
HOST=--specs=rdimon.specs -lgcc -lc -lrdimon --specs=nano.specs -nostartfiles
else
HOST=-lc -lgcc --specs=nosys.specs --specs=nano.specs -lrdimon -nostartfiles
endif

##########################################################################
# Compiler settings, parameters and flags
##########################################################################

# Options for specific architecture
ARCH_FLAGS=-mthumb -mcpu=$(CPU_TYPE)

# Link for code size
GC=-Wl,--gc-sections

# Create map file
MAP=-Wl,-Map=$(OLOC)/$(OUTFILE).map,--cref


CFLAGS =  $(ARCH_FLAGS) $(STARTUP_DEFS) $(OPT_LEVEL) $(DEBUG_OPTS) -DTARGET=$(TARGET) -D$(TARGET) \
		-ffunction-sections -fdata-sections -Wdouble-promotion -Wall $(GCC_DEFINE)
ASFLAGS = -c $(DEBUG_OPTS) $(INCLUDE_PATHS) $(ARCH_FLAGS) $(GCC_DEFINE) \
          -x assembler-with-cpp
LDFLAGS = $(CFLAGS) $(ARCH_FLAGS) -Wl,--no-wchar-size-warning,--gc-sections $(MAP) $(HOST)

OCFLAGS = --strip-unneeded

OBJS =  $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.s,%.o,$(SFILES))
POBJS = $(patsubst %,$(OLOC)/%,$(OBJS))
PDEPS =$(POBJS:.o=.d)

all : build

$(OLOC)/%.o : %.c
	$(Q)mkdir -p $(basename $@)
	$(call cmd, \$(CC) -c $(CFLAGS) $(PROFILING) $(INCLUDE_PATHS) -MMD -o $@ $< ,\
	Compiling $<)

$(OLOC)/%.o : %.s
	$(Q)mkdir -p $(basename $@)
	$(call cmd, \$(AS) $(ASFLAGS) -o  $@ $< ,\
	Assembling $<)

$(OLOC)/%.o : %.S
	$(Q)mkdir -p $(basename $@)
	$(call cmd, \$(AS) $(ASFLAGS) -o  $@ $< ,\
	Assembling $<)

build:  $(POBJS) $(SYS_OBJS) 
	@echo " Built $(VARIANT) version"
	$(Q)$(LD) -g $(LDFLAGS) -T $(LD_SCRIPT) $(MAP) $(POBJS) $(LDLIBS) -o $(OLOC)/$(OUTFILE).elf
	$(Q)$(SIZE) $(OLOC)/$(OUTFILE).elf
	$(Q)$(OBJCOPY) $(OCFLAGS) -O binary $(OLOC)/$(OUTFILE).elf $(OLOC)/$(OUTFILE).bin
	$(Q)$(OBJCOPY) $(OCFLAGS) -O ihex $(OLOC)/$(OUTFILE).elf $(OLOC)/$(OUTFILE).hex

tags:
	-@etags $(CFILES) 2> /dev/null

clean:
	-$(call cmd, \rm -f $(POBJS) $(LD_TEMP) $(OUTFILE).elf $(OUTFILE).bin $(OUTFILE).hex $(OUTFILE).map $(EXPORT) ,\
	Cleaning )
	$(Q)-rm -rf *~ core
	$(Q)-rm -rf $(OLOC)/*
	$(Q)-rm -rf config/*~
	$(Q)-rm -rf TAGS

flash:	build
	$(Q) $(DEBUGGER)   $(DEBUGSCRIPT) 2> /dev/null &

print-%:
	@echo $* is $($*)

pretty:
	$(Q)-$(STYLE) -i -style=file $(CFILES)

-include $(PDEPS)
