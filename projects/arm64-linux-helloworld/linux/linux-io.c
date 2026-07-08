#include <linux/linux.h>

uint8_t linux_io_read8(io_addr_t addr)
{
	return (*((volatile uint8_t *)(addr)));
}

void linux_io_write8(io_addr_t addr, uint8_t value)
{
	*((volatile uint8_t *)(addr)) = value;
}

uint16_t linux_io_read16(io_addr_t addr)
{
	return (*((volatile uint16_t *)(addr)));
}

void linux_io_write16(io_addr_t addr, uint16_t value)
{
	*((volatile uint16_t *)(addr)) = value;
}

uint32_t linux_io_read32(io_addr_t addr)
{
	return (*((volatile uint32_t *)(addr)));
}

void linux_io_write32(io_addr_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

uint64_t linux_io_read64(io_addr_t addr)
{
	return (*((volatile uint64_t *)(addr)));
}

void linux_io_write64(io_addr_t addr, uint64_t value)
{
	*((volatile uint64_t *)(addr)) = value;
}
