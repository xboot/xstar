OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

STACK_SIZE = 0x100000;

MEMORY
{
	ram  : org = 0x80000000, len = 8M
	dma  : org = 0x80800000, len = 8M
	heap : org = 0x81000000, len = 48M
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
		*(.rodata*)
		*(.srodata*)
		PROVIDE(__rodata_end = .);
	} > ram

	.data ALIGN(8) :
	{
		PROVIDE(__data_start = .);
		PROVIDE(__global_pointer$ = . + 0x800);
		*(.sdata*)
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
		. += STACK_SIZE;
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
