# ==========================================
# Makefile cho STM32F103C8T6
# ARM GNU Toolchain
# ==========================================

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CPU = cortex-m3

BUILD_DIR = build
TARGET = bai3

# ==========================================
# Compiler flags
# ==========================================

CFLAGS = -mcpu=$(CPU) \
         -mthumb \
         -DSTM32F10X_MD \
         -I. \
         -I./corecm3 \
         -I./Libraries/CMSIS/CM3/CoreSupport \
         -I./Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x \
         -I./Libraries/STM32F10x_StdPeriph_Driver/inc \
         -Wall \
         -ffunction-sections \
         -fdata-sections

# ==========================================
# Linker flags
# ==========================================

LDFLAGS = -mcpu=$(CPU) \
          -mthumb \
          -T linker.ld \
          -nostdlib

# ==========================================
# Source files
# ==========================================

SRCS = main.c \
       system_stm32f10x.c

STARTUP = startup_stm32f10x_md.s

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o) \
       $(BUILD_DIR)/startup_stm32f10x_md.o

# ==========================================
# Build
# ==========================================

all: $(BUILD_DIR)/$(TARGET).elf \
     $(BUILD_DIR)/$(TARGET).bin

# ==========================================
# Compile C files
# ==========================================

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# Compile startup
# ==========================================

$(BUILD_DIR)/startup_stm32f10x_md.o: $(STARTUP)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# Link
# ==========================================

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) \
	$(OBJS) \
	-Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
	-o $@
	$(SIZE) $@

# ==========================================
# Convert ELF -> BIN
# ==========================================

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# ==========================================
# Flash STM32
# ==========================================

flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x08000000

# ==========================================
# Clean
# ==========================================

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all flash clean
