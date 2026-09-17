##############################################################################
# Makefile cho STM32F10x (Standard Peripheral Library) - biên dịch trên Linux
##############################################################################

# ============== 1. CẤU HÌNH CHIP ==============
DEVICE_DEFS = -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

# Tên file output
TARGET = main

# ============== 2. TOOLCHAIN ==============
PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
AS      = $(PREFIX)gcc
LD      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE    = $(PREFIX)size

# ============== 3. THƯ MỤC ==============
CMSIS_CORE   = Libraries/CMSIS/CM3/CoreSupport
CMSIS_DEVICE = Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
SPL_INC      = Libraries/STM32F10x_StdPeriph_Driver/inc
SPL_SRC      = Libraries/STM32F10x_StdPeriph_Driver/src

# Đường dẫn chính xác theo kết quả find của bạn:
STARTUP_DIR  = $(CMSIS_DEVICE)/startup/gcc_ride7
STARTUP_FILE = $(STARTUP_DIR)/startup_stm32f10x_md.s

# Linker script
LDSCRIPT = linker.ld

# ============== 4. FILE NGUỒN ==============
C_SOURCES = main.c
C_SOURCES += $(CMSIS_DEVICE)/system_stm32f10x.c
C_SOURCES += $(wildcard $(SPL_SRC)/*.c)

# ============== 5. INCLUDE PATH ==============
INCLUDES  = -I.
INCLUDES += -I$(CMSIS_CORE)
INCLUDES += -I$(CMSIS_DEVICE)
INCLUDES += -I$(SPL_INC)

# ============== 6. CỜ BIÊN DỊCH ==============
MCU_FLAGS = -mcpu=cortex-m3 -mthumb

CFLAGS  = $(MCU_FLAGS) $(DEVICE_DEFS) $(INCLUDES)
CFLAGS += -Wall -O2 -g -std=gnu99
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -MMD -MP

ASFLAGS = $(MCU_FLAGS) -x assembler-with-cpp

LDFLAGS  = $(MCU_FLAGS) -T$(LDSCRIPT)
LDFLAGS += -nostartfiles -Wl,--gc-sections
LDFLAGS += -specs=nano.specs -specs=nosys.specs
LDFLAGS += -Wl,-Map=$(TARGET).map

# ============== 7. OBJECT FILES ==============
OBJDIR = build

C_OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(C_SOURCES:.c=.o)))
STARTUP_OBJ = $(OBJDIR)/startup_stm32f10x_md.o

OBJECTS = $(C_OBJECTS) $(STARTUP_OBJ)

# VPATH giúp make tìm file .c ở đúng thư mục con tương ứng
VPATH = . $(CMSIS_DEVICE) $(SPL_SRC)

# ============== 8. RULES ==============
all: $(OBJDIR) $(TARGET).elf $(TARGET).hex $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Rule biên dịch file .c (dùng VPATH)
$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Rule RIÊNG cho file startup (đường dẫn tường minh, không phụ thuộc VPATH)
$(STARTUP_OBJ): $(STARTUP_FILE)
	$(AS) -c $(ASFLAGS) $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(OBJDIR) $(TARGET).elf $(TARGET).hex $(TARGET).bin $(TARGET).map

# ============== 9. NẠP CODE ==============
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x8000000

flash-openocd: $(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "program $(TARGET).elf verify reset exit"

-include $(wildcard $(OBJDIR)/*.d)

.PHONY: all clean flash flash-openocd
