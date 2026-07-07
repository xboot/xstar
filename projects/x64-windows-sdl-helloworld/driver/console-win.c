#include <win/win.h>

static ssize_t console_win_read(struct console_t * console, unsigned char * buf, size_t count)
{
	return win_stdio_read((void *)buf, count);
}

static ssize_t console_win_write(struct console_t * console, const unsigned char * buf, size_t count)
{
	return win_stdio_write((void *)buf, count);
}

static struct device_t * console_win_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct console_t * console;
	struct device_t * dev;

	console = xos_mem_malloc(sizeof(struct console_t));
	if(!console)
		return NULL;

	console->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	console->read = console_win_read;
	console->write = console_win_write;
	console->priv = NULL;

	if(!(dev = register_console(console, drv)))
	{
		free_device_name(console->name);
		xos_mem_free(console);
		return NULL;
	}
	return dev;
}

static void console_win_remove(struct device_t * dev)
{
	struct console_t * console = (struct console_t *)dev->priv;

	if(console)
	{
		unregister_console(console);
		free_device_name(console->name);
		xos_mem_free(console);
	}
}

static void console_win_suspend(struct device_t * dev)
{
}

static void console_win_resume(struct device_t * dev)
{
}

static struct driver_t console_win = {
	.name		= "console-win",
	.probe		= console_win_probe,
	.remove		= console_win_remove,
	.suspend	= console_win_suspend,
	.resume		= console_win_resume,
};

static void console_win_driver_init(void)
{
	register_driver(&console_win);
}

static void console_win_driver_exit(void)
{
	unregister_driver(&console_win);
}

driver_initcall(console_win_driver_init);
driver_exitcall(console_win_driver_exit);
