# TODO решение с ifdef BUILD_DIR явно не лучшее
######################### Project Settings #########################
PRJ_NAME = IR_PCB
# What to include, in form dir1 dir2 dir3...
INCLUDE_DIRS = ./ Audio Filesys lib usb yartos Lapki
# What to define in form MYDEF1 MYDEF2=18 MYDEF3... $(MAKECMDGOALS) is name of requested action
DEFINS = 
DEFINS += BUILD_CFG_$(GOAL_NAME)=1 # add BUILD_CFG_GOALNAME=1 define, uppercasing GOAL_NAME
DEFINS += _GLIBCXX_FREESTANDING_EXTRA # new arm-none-eabi version prohibit stl compilation in freestangind mode(we use newlib)
# DEFINS += APP LEDS

######################### Figure out what to do #########################
# Target must be in the following form: build_Release, clean_Debug, flash_Fromboot
# The first word is the action (build, clean, flash); 
# the second is the name of the configuration and the name of the OUT_DIR.
INPUT_WORDS = $(subst _, ,$(MAKECMDGOALS)) # Replace '_' with ' '
ACTION = $(word 1,$(INPUT_WORDS))
GOAL_NAME = $(word 2,$(INPUT_WORDS))
ifeq ($(OUT_DIR),)
	OUT_DIR = $(GOAL_NAME)
endif
# Run Action (build_Release: build  clean_Debug: clean  flash_Fromboot: flash)
$(MAKECMDGOALS): $(ACTION)
##################
EXTRA_FLAGS ?= 
EXTRA_DIR ?= 
COPY_DIR ?=
ifneq ($(EXTRA_DIR),)
	EXTRA_INCLUDE = -I$(EXTRA_DIR)
	vpath %.c $(EXTRA_DIR)
	vpath %.h $(EXTRA_DIR)
	vpath %.cpp $(EXTRA_DIR)
endif
COMMON_FLAGS += $(EXTRA_FLAGS)

######################### Common Cfg Settings #########################
MCU = cortex-m4
FLOAT_FLAGS = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion -Wfloat-conversion -Wfloat-equal
WARNING_FLAGS = -Wall -Wlogical-op -Werror
DISABLED_WARNINGS = -Wno-address-of-packed-member -Wno-unknown-pragmas
COMMON_FLAGS = -mcpu=$(MCU) -mthumb -fmessage-length=0 -ffunction-sections -fdata-sections  $(FLOAT_FLAGS) $(WARNING_FLAGS) #-ffreestanding
CPP_FLAGS = -std=gnu++17 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics $(DISABLED_WARNINGS)
C_FLAGS = -std=gnu17
# Add this to build commands. Nothing to change here.
ifeq ($(OS),Windows_NT)
	OBJ_FLAGS = -MMD -MP -MF"$(subst /,\,$(@:%.o=%.d))" -MT"$@" -c -o "$@" "$<"
else
   	OBJ_FLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
endif
ifdef BUILD_DIR
LINKER_FLAGS = -Xlinker --gc-sections --specs=nano.specs --specs=nosys.specs -nostartfiles -Wl,-Map,"$(BUILD_DIR)/$(PRJ_NAME).map" -o "$(BUILD_DIR)/$(PRJ_NAME).elf"
else
LINKER_FLAGS = -Xlinker --gc-sections --specs=nano.specs --specs=nosys.specs -nostartfiles -Wl,-Map,"$(OUT_DIR)/$(PRJ_NAME).map" -o "$(OUT_DIR)/$(PRJ_NAME).elf"
endif
######################### Individual Cfg Settings #########################
####### Release Cfg Settings #######
ifeq "$(GOAL_NAME)" "Release"
COMMON_FLAGS += -Os -flto
LINKER_FLAGS += 
LD_SCRIPT = GD32E103xB.ld
# Comment / uncomment the following lines to produce .hex and/or .bin output
BUILD_HEX = $(OUT_DIR)/$(PRJ_NAME).hex
BUILD_BIN = $(OUT_DIR)/$(PRJ_NAME).bin



####### Debug Cfg Settings #######
else ifeq "$(GOAL_NAME)" "Debug"
COMMON_FLAGS += -O0 -g3
LINKER_FLAGS += 
LD_SCRIPT = GD32E103xB.ld
# Comment / uncomment the following lines to produce .hex and/or .bin output
BUILD_HEX = $(OUT_DIR)/$(PRJ_NAME).hex
BUILD_BIN = $(OUT_DIR)/$(PRJ_NAME).bin

####### FromBoot Cfg Settings ####### 
# Put to flash starting from 0x800XXXX, to reserve place for bootloader. All other is same as Release.
else ifeq "$(GOAL_NAME)" "Fromboot"
COMMON_FLAGS += -Os -flto
LINKER_FLAGS += 
LD_SCRIPT = GD32E103xB_Boot.ld
# Comment / uncomment the following lines to produce .hex and/or .bin output
ifdef BUILD_DIR
BUILD_HEX = $(BUILD_DIR)/$(PRJ_NAME).hex
else
BUILD_HEX = $(OUT_DIR)/$(PRJ_NAME).hex
endif
#BUILD_BIN was superseded by BIN_PATH
#BUILD_BIN = $(OUT_DIR)/$(PRJ_NAME).bin
BIN_PATH = $(if $(BUILD_DIR),$(BUILD_DIR)/fw_$(PRJ_NAME).bin, $(OUT_DIR)/fw_$(PRJ_NAME).bin)
endif

######################### Toolchain ######################### 
CPP_CMP = arm-none-eabi-g++
C_CMP = arm-none-eabi-gcc
OBJCPY = arm-none-eabi-objcopy
SZ = arm-none-eabi-size
GDB = arm-none-eabi-gdb  # Required for flashing using BMP
# GDB COM port: required for flashing using BMP
GDB_COM = :3333 #\\.\COM11

######################### Do not touch #########################
.PHONY: .FORCE print_size clean flash # "virtual" symbols to "rebuild" them always
# Current datetime
TIMESTAMP = $(shell "date" "+%Y%m%d_%H%M")
# Build include flag string out of INCLUDE_DIRS list
INCLUDE_STR = $(addprefix -I,$(INCLUDE_DIRS))
Libraries_DIRS_STR = $(addprefix -L,$(LIBRARIES_DIRS))
Libraries_STR = $(addprefix -l,$(LIBRARIES))

# Build define flag string out of DEFINS list, surrounding with double quotes
ToUppercase = $(shell echo $(1) | tr '[:lower:]' '[:upper:]')
DEFINE_STR = $(patsubst %,-D"%",$(DEFINS))
COMMON_FLAGS += $(DEFINE_STR)
# Recursive wildcard to iterate subdirs of any depth
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
# Find all .cpp, .c and .S files in all subfolders of ../ 
SRCS = $(call rwildcard,./,*.c) $(call rwildcard,./,*.cpp) $(call rwildcard,./,*.S)
ifneq ($(EXTRA_DIR),)
	SRCS += qhsm.cpp
endif
# Remove leading ./ 
SRCS := $(patsubst ./%,%,$(SRCS))
# Replace .cpp, .c, .S with .o
OBJS = $(SRCS:.cpp=.o) # Take all srcs replacing .cpp with .o
OBJS := $(OBJS:.c=.o)  # Take all objs replacing .c with .o
OBJS := $(OBJS:.S=.o)  # Take all objs replacing .S with .o
# Find all source-containing folders; remove ending /; remove .
SUBDIRS = $(filter-out .,$(patsubst %/,%,$(sort $(dir $(SRCS)))))
# Search src here. Nothing to change here.
VPATH := ../

######################### Build #########################
# Add OUT_DIR prefix to OBJS
OBJS := $(addprefix $(OUT_DIR)/, $(OBJS))
# Include dependents *.d (ignore if not exist) to rebuild what depends on changed .h
ifdef BUILD_DIR
    OBJS := $(patsubst $(OUT_DIR)/main.o, $(BUILD_DIR)/main.o, $(OBJS))
endif

-include $(filter-out $(OUT_DIR)/main.d, $(OBJS:.o=.d))

# Build: Require dir tree, .elf file, .hex, .bin, .siz
ifdef BUILD_DIR
build: $(OUT_DIR)/out_subdirs $(BUILD_DIR)/$(PRJ_NAME).elf $(BUILD_HEX) $(BIN_PATH) print_size 
else
build: $(OUT_DIR)/out_subdirs $(OUT_DIR)/$(PRJ_NAME).elf $(BUILD_HEX) $(BIN_PATH) print_size 
endif
# $(info SRCS: $(SRCS))
# $(info OBJS: $(OBJS))
# Construct dir tree: out_dir/dir1, out_dir/dir2, ...
$(OUT_DIR)/out_subdirs:
#	@mkdir -p $(addprefix $(OUT_DIR)/, $(SUBDIRS))
	@mkdir -p $(OUT_DIR)
	@if [ -n "$(strip $(SUBDIRS))" ]; then \
		mkdir -p $(addprefix $(OUT_DIR)/, $(SUBDIRS)); \
	fi

# Construct .elf
ifdef BUILD_DIR
$(BUILD_DIR)/$(PRJ_NAME).elf: $(OBJS)
	@echo 'EXTRA FLAGS: $(EXTRA_FLAGS)'
	@echo 'Linking $@'
	@$(CPP_CMP) $(COMMON_FLAGS) -T $(LD_SCRIPT) $(LINKER_FLAGS) $(OBJS)
else
$(OUT_DIR)/$(PRJ_NAME).elf: $(OBJS)
	@echo 'EXTRA FLAGS: $(EXTRA_FLAGS)'
	@echo 'Linking $@'
	@$(CPP_CMP) $(COMMON_FLAGS) -T $(LD_SCRIPT) $(LINKER_FLAGS) $(OBJS)
endif
### Compile objs ###
# Special rule for version.cpp: always rebuild
VERSION_DEFINS = -D"BUILD_TIME=$(TIMESTAMP)" -D"BUILD_CFG=$(call ToUppercase,$(GOAL_NAME))"
$(OUT_DIR)/version.o: version.cpp .FORCE
	@echo 'Building $<'
	@$(CPP_CMP) $(COMMON_FLAGS) $(VERSION_DEFINS) $(INCLUDE_STR) $(CPP_FLAGS) $(OBJ_FLAGS)

#always rebuild main
ifdef BUILD_DIR
$(BUILD_DIR)/main.o: main.cpp .FORCE
	@echo "Building $< -> $@ (BUILD_DIR: $(BUILD_DIR))"
	@echo '$(OUT_DIR)'
	@echo 'Building $<'
	@mkdir -p $(BUILD_DIR)
	@$(CPP_CMP) $(EXTRA_INCLUDE) $(COMMON_FLAGS) $(VERSION_DEFINS) $(INCLUDE_STR) $(CPP_FLAGS) -c -o "$@" "$<"
else
$(OUT_DIR)/main.o: main.cpp .FORCE
	@echo 'Building $<'
	@$(CPP_CMP) $(EXTRA_INCLUDE) $(COMMON_FLAGS) $(VERSION_DEFINS) $(INCLUDE_STR) $(CPP_FLAGS) -c -o "$@" "$<"
endif
# cpp
$(OUT_DIR)/%.o: %.cpp
	@echo 'Building $<'
	@$(CPP_CMP) $(EXTRA_INCLUDE) $(COMMON_FLAGS) $(INCLUDE_STR) $(CPP_FLAGS) $(OBJ_FLAGS) 
# c
$(OUT_DIR)/%.o: %.c
	@echo 'Building $<'
	@$(C_CMP) $(EXTRA_INCLUDE) $(COMMON_FLAGS) $(INCLUDE_STR) $(C_FLAGS) $(OBJ_FLAGS)
# S
$(OUT_DIR)/%.o: %.S
	@echo 'Building $<'
	@$(C_CMP) -x assembler-with-cpp $(COMMON_FLAGS) $(INCLUDE_STR) $(OBJ_FLAGS) 

# Output .hex
ifdef BUILD_DIR
$(BUILD_DIR)/$(PRJ_NAME).hex: $(BUILD_DIR)/$(PRJ_NAME).elf
	@echo 'Constructing $@'
	@$(OBJCPY) -O ihex "$(BUILD_DIR)/$(PRJ_NAME).elf" "$(BUILD_DIR)/$(PRJ_NAME).hex"
else
$(OUT_DIR)/$(PRJ_NAME).hex: $(OUT_DIR)/$(PRJ_NAME).elf
	@echo 'Constructing $@'
	@$(OBJCPY) -O ihex "$(OUT_DIR)/$(PRJ_NAME).elf" "$(OUT_DIR)/$(PRJ_NAME).hex"
endif
# Output .bin
ifdef BUILD_DIR
$(BIN_PATH): $(BUILD_DIR)/$(PRJ_NAME).elf
	@echo 'Constructing $@'
	@$(OBJCPY) -O binary "$(BUILD_DIR)/$(PRJ_NAME).elf" $(BIN_PATH)
#	@echo 'copydir : $(COPY_DIR)'
	@echo 'EXTRA_DIR: $(EXTRA_DIR)'
else
$(BIN_PATH): $(OUT_DIR)/$(PRJ_NAME).elf
	@echo 'Constructing $@'
	@$(OBJCPY) -O binary "$(OUT_DIR)/$(PRJ_NAME).elf" $(BIN_PATH)
#	@echo 'copydir : $(COPY_DIR)'
	@echo 'EXTRA_DIR: $(EXTRA_DIR)'
endif

# Print size
print_size: 
	@echo 'Size:'
ifdef BUILD_DIR
	@$(SZ) --format=berkeley "$(BUILD_DIR)/$(PRJ_NAME).elf"
else
	@$(SZ) --format=berkeley "$(OUT_DIR)/$(PRJ_NAME).elf"
endif

######################### Clean #########################
clean:
	@rm -rf $(OUT_DIR)
	@echo 'Done.'


######################### Flash it ######################### Using BlackMagicProbe
flash:
	$(GDB) -q -ex "target extended-remote $(GDB_COM)" -ex "mon swdp_scan" -ex "att 1" -ex "load $(OUT_DIR)/$(PRJ_NAME).hex" -ex "det" -ex "quit"


######################### Test for debugging ########################
.PHONY: test
test: 
	@echo $(ACTION)
	@echo $(OUT_DIR)
	@echo "VPATH: $(VPATH)"
	@echo "EXTRA_DIR: $(EXTRA_DIR)"
	@echo "SHELL: $(SHELL)"
	@echo "OS: $(OS)"
	@if [ -n "$$" ]; then echo "Успех! Используется sh-совместимая оболочка."; else echo "Ошибка! Используется cmd.exe."; fi