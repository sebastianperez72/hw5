/* Todo contiguo desde 0x0 (el programa no debe pisar 0x1000, donde van los datos de prueba) */
ENTRY(_start)
SECTIONS {
  . = 0x0;
  .text : { *(.text.start) *(.text*) *(.rodata*) *(.srodata*) *(.data*) *(.sdata*) *(.bss*) *(.sbss*) }
}
