#include <linux/linux.h>

struct uart_linux_pdata_t {
	int fd;
	int baud;
	int data;
	int parity;
	int stop;
};

static int uart_linux_set(struct uart_t * uart, int baud, int data, int parity, int stop)
{
	struct uart_linux_pdata_t * pdat = (struct uart_linux_pdata_t *)uart->priv;

	if(baud < 0)
		return FALSE;
	if((data < 5) || (data > 8))
		return FALSE;
	if((parity < 0) || (parity > 2))
		return FALSE;
	if((stop < 0) || (stop > 2))
		return FALSE;

	pdat->baud = baud;
	pdat->data = data;
	pdat->parity = parity;
	pdat->stop = stop;
	linux_uart_set(pdat->fd, pdat->baud, pdat->data, pdat->parity, pdat->stop);

	return TRUE;
}

static int uart_linux_get(struct uart_t * uart, int * baud, int * data, int * parity, int * stop)
{
	struct uart_linux_pdata_t * pdat = (struct uart_linux_pdata_t *)uart->priv;

	if(baud)
		*baud = pdat->baud;
	if(data)
		*data = pdat->data;
	if(parity)
		*parity = pdat->parity;
	if(stop)
		*stop = pdat->stop;
	return TRUE;
}

static ssize_t uart_linux_read(struct uart_t * uart, uint8_t * buf, size_t count)
{
	struct uart_linux_pdata_t * pdat = (struct uart_linux_pdata_t *)uart->priv;
	return linux_uart_read(pdat->fd, buf ,count);
}

static ssize_t uart_linux_write(struct uart_t * uart, const uint8_t * buf, size_t count)
{
	struct uart_linux_pdata_t * pdat = (struct uart_linux_pdata_t *)uart->priv;
	return linux_uart_write(pdat->fd, buf, count);
}

static struct device_t * uart_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct uart_linux_pdata_t * pdat;
	struct uart_t * uart;
	struct device_t * dev;
	int fd = linux_uart_open(dt_read_string(n, "device", NULL));

	if(fd < 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct uart_linux_pdata_t));
	if(!pdat)
		return NULL;

	uart = xos_mem_malloc(sizeof(struct uart_t));
	if(!uart)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->fd = fd;
	pdat->baud = dt_read_int(n, "baud-rates", 115200);
	pdat->data = dt_read_int(n, "data-bits", 8);
	pdat->parity = dt_read_int(n, "parity-bits", 0);
	pdat->stop = dt_read_int(n, "stop-bits", 1);

	uart->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	uart->set = uart_linux_set;
	uart->get = uart_linux_get;
	uart->read = uart_linux_read;
	uart->write = uart_linux_write;
	uart->priv = pdat;
	uart_linux_set(uart, pdat->baud, pdat->data, pdat->parity, pdat->stop);

	if(!(dev = register_uart(uart, drv)))
	{
		linux_uart_close(pdat->fd);
		free_device_name(uart->name);
		xos_mem_free(uart->priv);
		xos_mem_free(uart);
		return NULL;
	}
	return dev;
}

static void uart_linux_remove(struct device_t * dev)
{
	struct uart_t * uart = (struct uart_t *)dev->priv;
	struct uart_linux_pdata_t * pdat = (struct uart_linux_pdata_t *)uart->priv;

	if(uart)
	{
		unregister_uart(uart);
		linux_uart_close(pdat->fd);
		free_device_name(uart->name);
		xos_mem_free(uart->priv);
		xos_mem_free(uart);
	}
}

static void uart_linux_suspend(struct device_t * dev)
{
}

static void uart_linux_resume(struct device_t * dev)
{
}

static struct driver_t uart_linux = {
	.name		= "uart-linux",
	.probe		= uart_linux_probe,
	.remove		= uart_linux_remove,
	.suspend	= uart_linux_suspend,
	.resume		= uart_linux_resume,
};

static void uart_linux_driver_init(void)
{
	register_driver(&uart_linux);
}

static void uart_linux_driver_exit(void)
{
	unregister_driver(&uart_linux);
}

driver_initcall(uart_linux_driver_init);
driver_exitcall(uart_linux_driver_exit);
