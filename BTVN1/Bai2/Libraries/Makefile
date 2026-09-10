# ==============================================================================
# TOOLCHAIN DEFINITIONS
# ==============================================================================
CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc -x assembler-with-cpp
CP      = arm-none-eabi-objcopy
SZ      = arm-none-eabi-size
HEX     = $(CP) -O ihex
BIN     = $(CP) -O binary -S

# ==============================================================================
# PROJECT CONFIGURATION
# ==============================================================================
TARGET    = firmware
BUILD_DIR = build
MCU_DEF   = -DSTM32F10X_MD
MCU       = -mcpu=cortex-m3 -mthumb

# ==============================================================================
# PATHS & SOURCES
# ==============================================================================
# Đường dẫn thư mục chứa file startup
STARTUP_DIR = Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/TrueSTUDIO

# Khai báo cho make biết nơi tìm các file mã nguồn (.c và .s)
vpath %.c . Src Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
vpath %.s $(STARTUP_DIR)

# Danh sách mã nguồn
SRCS_C   = main.c system_stm32f10x.c
SRCS_ASM = startup_stm32f10x_md.s

# Thư mục chứa header (.h)
INC_DIRS  = . Inc Src \
            Libraries/CMSIS/CM3/CoreSupport \
            Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
C_INCLUDES = $(addprefix -I, $(INC_DIRS))

# ==============================================================================
# COMPILER & LINKER FLAGS
# ==============================================================================
CFLAGS   = $(MCU) $(MCU_DEF) $(C_INCLUDES) -O0 -Wall -fdata-sections -ffunction-sections -g -gdwarf-2
LDSCRIPT = linker.ld
LDFLAGS  = $(MCU) -T$(LDSCRIPT) -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs

# ==============================================================================
# OBJECTS
# ==============================================================================
OBJECTS  = $(addprefix $(BUILD_DIR)/, $(SRCS_C:.c=.o))
OBJECTS += $(addprefix $(BUILD_DIR)/, $(SRCS_ASM:.s=.o))

# ==============================================================================
# BUILD TARGETS
# ==============================================================================
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# Biên dịch file C
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "[CC]  $<"
	@$(CC) -c $(CFLAGS) $< -o $@

# Biên dịch file Assembly từ thư mục STARTUP_DIR
$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@echo "[AS]  $<"
	@$(AS) -c $(CFLAGS) $< -o $@

# Liên kết (Linker)
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	@echo "[LD]  $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

# ==============================================================================
# FLASH & CLEAN
# ==============================================================================
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x8000000

reset:
	st-flash reset

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean flash reset
