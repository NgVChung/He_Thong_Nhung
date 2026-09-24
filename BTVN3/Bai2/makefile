
TARGET = Bai2

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
STFLASH = st-flash

LIB = /home/hao/stm32/libraries/stm32f10x-stdperiph-lib

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
         -mfloat-abi=soft \
         -DSTM32F10X_MD \
         -DUSE_STDPERIPH_DRIVER \
         -O0 \
         -g \
         -Wall

INCLUDES = -I. \
           -I$(LIB)/Libraries/CMSIS/CM3/CoreSupport \
           -I$(LIB)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x \
           -I$(LIB)/Libraries/STM32F10x_StdPeriph_Driver/inc

LDFLAGS = -Tlinker.ld \
          -mcpu=cortex-m3 \
          -mthumb \
          -mfloat-abi=soft \
          -nostartfiles \
          -nostdlib

STD_OBJ = stm32f10x_gpio.o \
          stm32f10x_rcc.o \
          stm32f10x_spi.o

all: $(TARGET).bin

$(TARGET).elf: main.o startup_stm32f10x_md.o $(STD_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

startup_stm32f10x_md.o: startup_stm32f10x_md.s
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

stm32f10x_gpio.o: $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

stm32f10x_rcc.o: $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

stm32f10x_spi.o: $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	$(STFLASH) write $(TARGET).bin 0x08000000

clean:
	rm -f *.o *.elf *.bin

