#include <linux/linux.h>

int linux_spi_exist(const char * dev)
{
	struct stat st;

	if((stat(dev, &st) == 0) && S_ISCHR(st.st_mode) && (access(dev, R_OK | W_OK) == 0))
		return 1;
	return 0;
}

int linux_spi_open(const char * dev)
{
	int fd;

	fd = open(dev, O_RDWR);
	if(fd > 0)
		return fd;
	return 0;
}

void linux_spi_close(int fd)
{
	if(fd > 0)
		close(fd);
}

void linux_spi_mode(int fd, int mode)
{
	if(fd > 0)
	{
		uint8_t value = (uint8_t)(mode & 0xff);
		ioctl(fd, SPI_IOC_WR_MODE, &value);
	}
}

int linux_spi_transfer(int fd, struct linux_spi_msg_t * msg)
{
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)msg->txbuf,
		.rx_buf = (unsigned long)msg->rxbuf,
		.len = msg->len,
		.speed_hz = msg->speed,
		.delay_usecs = 0,
		.bits_per_word = msg->bits,
		.cs_change = 0,
		.tx_nbits = msg->type,
		.rx_nbits = msg->type,
		.word_delay_usecs = 0,
	};
	return (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) ? 0 : msg->len;
}

void linux_spi_select(int fd, int cs)
{
}

void linux_spi_deselect(int fd, int cs)
{
}
