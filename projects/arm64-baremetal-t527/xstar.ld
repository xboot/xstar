OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH("aarch64")
ENTRY(_start)

STACK_EL3_SIZE = 0x40000;
STACK_EL2_SIZE = 0x40000;
STACK_EL1_SIZE = 0x100000;
STACK_EL0_SIZE = 0x100000;

MEMORY
{
	ram  : org = 0x40000000, len = 128M
	dma  : org = 0x48000000, len = 128M
	heap : org = 0x50000000, len = 256M
}

SECTIONS
{
	.text :
	{
		PROVIDE(__image_start = .);
		PROVIDE(__text_start = .);
		*(.text*)
		*(.iplt)
		*(.note.gnu.build-id)
		PROVIDE(__text_end = .);
	} > ram

	.rodata ALIGN(8) :
	{
		PROVIDE(__rodata_start = .);
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		PROVIDE(__rodata_end = .);
	} > ram

	.data ALIGN(8) :
	{
		PROVIDE(__data_start = .);
		*(.data*)
		. = ALIGN(8);
  		PROVIDE(__data_end = .);
		PROVIDE(__image_end = .);
	} > ram

	.bss ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__bss_start = .);
		*(.bss*)
		*(.sbss*)
		*(COMMON)
		. = ALIGN(8);
		PROVIDE(__bss_end = .);
	} > ram

	.stack ALIGN(16) (NOLOAD) :
	{
		PROVIDE(__stack_start = .);
		PROVIDE(__stack_el3_start = .);
		. += STACK_EL3_SIZE;
		PROVIDE(__stack_el3_end = .);
		. = ALIGN(16);
		PROVIDE(__stack_el2_start = .);
		. += STACK_EL2_SIZE;
		PROVIDE(__stack_el2_end = .);
		. = ALIGN(16);
		PROVIDE(__stack_el1_start = .);
		. += STACK_EL1_SIZE;
		PROVIDE(__stack_el1_end = .);
		. = ALIGN(16);		
		PROVIDE(__stack_el0_start = .);
		. += STACK_EL0_SIZE;
		PROVIDE(__stack_el0_end = .);
		. = ALIGN(16);
		PROVIDE(__stack_end = .);
	} > ram

	.dma ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__dma_start = ORIGIN(dma));
		PROVIDE(__dma_end = ORIGIN(dma) + LENGTH(dma));
	} > dma

	.heap ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__heap_start = ORIGIN(heap));
		PROVIDE(__heap_end = ORIGIN(heap) + LENGTH(heap));
	} > heap

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}
