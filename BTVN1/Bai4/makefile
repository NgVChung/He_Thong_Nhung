# --- Cấu hình Toolchain ---
PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
AS      = $(PREFIX)as
OBJCOPY = $(PREFIX)objcopy
SIZE    = $(PREFIX)size

# --- Cấu hình Vi điều khiển (STM32F103 Medium Density) ---
MCU     = -mcpu=cortex-m3 -mthumb

# --- Thư mục Include (.h) ---
INC_DIRS  = -I. -Icorecm3
INC_DIRS += -ILibraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
INC_DIRS += -ILibraries/CMSIS/CM3/CoreSupport

# Danh sách các file nguồn C và Assembly (.s)
# Bổ sung file system_stm32f10x.c vào danh sách dịch
SRCS_C   = main.c Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
SRCS_ASM = startup_stm32f10x_md.s

# Chuyển đổi tên file .c và .s thành .o tương ứng
OBJS     = $(SRCS_C:.c=.o) $(SRCS_ASM:.s=.o)

# --- Cờ biên dịch ---
# Định nghĩa dòng chip Medium Density (-DSTM32F10X_MD), bật debug (-g3), tắt tối ưu (-O0)
CFLAGS   = $(MCU) $(INC_DIRS) -DSTM32F10X_MD -O0 -g3 -Wall
ASFLAGS  = $(MCU)
LDFLAGS  = $(MCU) -T linker.ld -nostartfiles -Wl,--gc-sections

# --- Target chính ---
all: firmware.elf firmware.bin
	@echo "--- Dung lượng bộ nhớ ---"
	$(SIZE) firmware.elf

# Biên dịch các file .c thành .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Biên dịch file .s (Assembly) thành .o
%.o: %.s
	$(CC) $(ASFLAGS) -c $< -o $@

# Liên kết tạo file .elf
firmware.elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Chuyển đổi .elf sang .bin
firmware.bin: firmware.elf
	$(OBJCOPY) -O binary $< $@

# --- Lệnh Nạp Code ---
flash: firmware.bin
	st-flash write firmware.bin 0x08000000

# --- Dọn dẹp ---
clean:
	rm -f $(OBJS) firmware.elf firmware.bin

.PHONY: all flash clean
