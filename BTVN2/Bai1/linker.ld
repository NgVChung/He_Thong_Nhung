/* Định nghĩa điểm nhập của chương trình */
ENTRY(Reset_Handler)

/* Khai báo đỉnh của Stack (20KB RAM = 0x20000000 + 20K) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Khai báo bộ nhớ phần cứng của STM32F103C8T6 */
MEMORY
{
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
}

/* Định nghĩa các Section bộ nhớ */
SECTIONS
{
  /* 1. Bảng vectơ ngắt (ISR Vector Table) nằm đầu bộ nhớ Flash */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH

  /* 2. Mã chương trình (Code) và dữ liệu hằng số (Read-Only) */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* Mốc kết thúc vùng code trong Flash */
  } > FLASH

  /* 3. Dữ liệu hằng số (Read-Only Data) */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  /* 4. Nhãn lấy địa chỉ Flash bắt đầu của dữ liệu .data để Copy sang RAM */
  _sidata = LOADADDR(.data);

  /* 5. Vùng dữ liệu toàn cục có khởi tạo giá trị (Nằm ở RAM, tải từ FLASH) */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* Mốc bắt đầu vùng .data trong RAM */
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;        /* Mốc kết thúc vùng .data trong RAM */
  } > RAM AT > FLASH

  /* 6. Vùng dữ liệu toàn cục chưa khởi tạo (Nằm ở RAM, xóa về 0 khi Reset) */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;         /* Mốc bắt đầu vùng .bss trong RAM */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* Mốc kết thúc vùng .bss trong RAM */
    __bss_end__ = _ebss;
  } > RAM

  /* 7. Kiểm tra loại bỏ các rác bộ nhớ không dùng */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
