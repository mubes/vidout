#VERBOSE=1
#DEBUG ?= 1

#Define this to export LCD information over the ITM channel 
WITH_ORBLCD_MONITOR=1

CROSS_COMPILE ?= arm-none-eabi-

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
OPT_LEVEL = -O3
VARIANT = "Release"
endif

ifdef WITH_ORBLCD_MONITOR
GCC_DEFINE+=-DMONITOR_OUTPUT
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
INCLUDE_PATHS = $(OLOC) /home/dmarples/Develop/orb/orbuculum/Inc
DEBUG_OPTS = -g3 -gdwarf-2 -ggdb

##########################################################################
# Generic multi-project files
##########################################################################

# CMSIS Files
# ===========
CMSIS_DIR=thirdparty/CMSIS
INCLUDE_PATHS += $(CMSIS_DIR)/inc

CFILES += $(CMSIS_DIR)/src/core_cm3.c \
		  $(CMSIS_DIR)/src/system_stm32f10x.c
SFILES += $(CMSIS_DIR)/src/startup_stm32f10x_md.s

# Video Files
# ===========
VIDEO_DIR=vidout
INCLUDE_PATHS += $(VIDEO_DIR)
CFILES += $(VIDEO_DIR)/displayFile.c \
		  $(VIDEO_DIR)/rasterLine.c \
		  $(VIDEO_DIR)/itm_messages.c \
		  $(VIDEO_DIR)/vidout.c

##########################################################################
# Project-specific files
##########################################################################

# Main Files
# ==========
App_DIR=app
App_Inc_DIR=app
INCLUDE_PATHS += $(App_Inc_DIR)

CFILES += $(App_DIR)/main.c

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
ASFLAGS = -c $(DEBUG_OPTS) $(ARCH_FLAGS) -x assembler-with-cpp
LDFLAGS = $(CFLAGS) $(ARCH_FLAGS) -Wl,--no-wchar-size-warning,--gc-sections $(MAP) $(HOST)

OCFLAGS = --strip-unneeded

OBJS =  $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.s,%.o,$(SFILES))
POBJS = $(patsubst %,$(OLOC)/%,$(OBJS))
PDEPS =$(POBJS:.o=.d)

INCLUDE_FLAGS = $(foreach d, $(INCLUDE_PATHS), -I$d)

all : build

$(OLOC)/%.o : %.c
	$(Q)mkdir -p $(basename $@)
	$(call cmd, \$(CC) -c $(CFLAGS) $(PROFILING) $(INCLUDE_FLAGS) -MMD -o $@ $< ,\
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
	$(Q)$(LD) -g $(LDFLAGS) -T $(LD_SCRIPT) $(MAP) $(POBJS) $(LDLIBS) -o $(OLOC)/$(OUTFILE).elf
	$(Q)$(SIZE) $(OLOC)/$(OUTFILE).elf
	$(Q)$(OBJCOPY) $(OCFLAGS) -O binary $(OLOC)/$(OUTFILE).elf $(OLOC)/$(OUTFILE).bin
	$(Q)$(OBJCOPY) $(OCFLAGS) -O ihex $(OLOC)/$(OUTFILE).elf $(OLOC)/$(OUTFILE).hex
	@echo " Built $(VARIANT) version"

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
