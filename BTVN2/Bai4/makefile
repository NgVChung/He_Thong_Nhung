TARGET = BAI4

CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

LIB = /home/hao/stm32/libraries/stm32f10x-stdperiph-lib

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
         -mfloat-abi=soft \
         -DSTM32F10X_MD \
         -DUSE_STDPERIPH_DRIVER \
         -I. \
         -I$(LIB)/Libraries/CMSIS/CM3/CoreSupport \
         -I$(LIB)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x \
         -I$(LIB)/Libraries/STM32F10x_StdPeriph_Driver/inc

CFLAGS += -O0 -g -Wall

LDFLAGS = -Tlinker.ld \
          -mcpu=cortex-m3 \
          -mthumb \
          -mfloat-abi=soft \
          -nostdlib

SRC = main.c \
      system_stm32f10x.c \
      $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
      $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
      $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c

STARTUP = startup_stm32f10x_md.s

OBJ = $(SRC:.c=.o)
STARTUP_OBJ = startup_stm32f10x_md.o

all: $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJ) $(STARTUP_OBJ)
	$(CC) $(OBJ) $(STARTUP_OBJ) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(STARTUP_OBJ): $(STARTUP)
	$(AS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(STARTUP_OBJ) $(TARGET).elf $(TARGET).bin
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000
