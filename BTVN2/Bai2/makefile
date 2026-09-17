# Trình biên dịch và các công cụ
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# Tên file đầu ra
TARGET  = BAI2

# Kiến trúc và Cờ biên dịch (CFLAGS)
MCU     = -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
DEFINES = -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

# Đường dẫn đến các thư mục Header (.h)
INCLUDES = -I. \
           -I./Libraries/CMSIS/CM3/CoreSupport \
           -I./Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x \
           -I./Libraries/STM32F10x_StdPeriph_Driver/inc

CFLAGS  = $(MCU) $(DEFINES) $(INCLUDES) -O0 -g -Wall

# Cờ dành cho Linker (LDFLAGS)
LDFLAGS = -Tlinker.ld $(MCU) --specs=nano.specs --specs=nosys.specs

# Đường dẫn file Startup (.s)
STARTUP_SRC = Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/gcc_ride7/startup_stm32f10x_md.s

# Danh sách các file nguồn C (.c)
C_SRCS = main.c \
         Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c \
         Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c

# Danh sách các file Object (.o) tạo ra
OBJS = $(C_SRCS:.c=.o) startup_stm32f10x_md.o

# Quy tắc mặc định (Build tất cả)
all: $(TARGET).elf $(TARGET).bin $(TARGET).hex
	$(SIZE) $(TARGET).elf

# Tạo file .elf từ các file .o
$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Quy tắc dịch file .c sang .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Quy tắc dịch file Startup .s sang .o (Đã sửa đường dẫn đúng)
startup_stm32f10x_md.o: $(STARTUP_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Quy tắc tạo file .bin và .hex để nạp vào vi điều khiển
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

# Quy tắc dọn dẹp các file trung gian
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).hex

.PHONY: all clean
