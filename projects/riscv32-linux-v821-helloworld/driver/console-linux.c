#include <linux/linux.h>

static ssize_t console_linux_read(struct console_t * console, unsigned char * buf, size_t count)
{
	return linux_stdio_read((void *)buf, count);
}

static ssize_t console_linux_write(struct console_t * console, const unsigned char * buf, size_t count)
{
	return linux_stdio_write((void *)buf, count);
}

static struct device_t * console_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct console_t * console;
	struct device_t * dev;

	console = xos_mem_malloc(sizeof(struct console_t));
	if(!console)
		return NULL;

	console->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	console->read = console_linux_read;
	console->write = console_linux_write;
	console->priv = NULL;

	if(!(dev = register_console(console, drv)))
	{
		free_device_name(console->name);
		xos_mem_free(console);
		return NULL;
	}
	return dev;
}

static void console_linux_remove(struct device_t * dev)
{
	struct console_t * console = (struct console_t *)dev->priv;

	if(console)
	{
		unregister_console(console);
		free_device_name(console->name);
		xos_mem_free(console);
	}
}

static void console_linux_suspend(struct device_t * dev)
{
}

static void console_linux_resume(struct device_t * dev)
{
}

static struct driver_t console_linux = {
	.name		= "console-linux",
	.probe		= console_linux_probe,
	.remove		= console_linux_remove,
	.suspend	= console_linux_suspend,
	.resume		= console_linux_resume,
};

static void console_linux_driver_init(void)
{
	register_driver(&console_linux);
}

static void console_linux_driver_exit(void)
{
	unregister_driver(&console_linux);
}

driver_initcall(console_linux_driver_init);
driver_exitcall(console_linux_driver_exit);
