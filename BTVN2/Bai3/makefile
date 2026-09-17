CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

TARGET  = BAI3
BUILD_DIR = build

MCU     = -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
DEFINES = -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

INCLUDES = -I. \
           -I./Libraries/CMSIS/CM3/CoreSupport \
           -I./Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x \
           -I./Libraries/STM32F10x_StdPeriph_Driver/inc

CFLAGS  = $(MCU) $(DEFINES) $(INCLUDES) -O0 -g -Wall
LDFLAGS = -Tlinker.ld $(MCU) --specs=nano.specs --specs=nosys.specs

# Danh sách file nguồn .c
C_SRCS = main.c \
         Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c

STARTUP_SRC = Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/gcc_ride7/startup_stm32f10x_md.s

# Chuyển hướng các file .o vào thư mục build/
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SRCS:.c=.o)))
vpath %.c $(sort $(dir $(C_SRCS)))

OBJS += $(BUILD_DIR)/startup_stm32f10x_md.o

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex
	$(SIZE) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/startup_stm32f10x_md.o: $(STARTUP_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
