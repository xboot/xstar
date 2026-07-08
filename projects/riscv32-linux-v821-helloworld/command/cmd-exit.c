#include <linux/linux.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    exit\r\n");
}

static int do_exit(int argc, char ** argv)
{
	linux_exit();
	return 0;
}

static struct command_t cmd_exit = {
	.name	= "exit",
	.desc	= "exit the xstar program",
	.usage	= usage,
	.exec	= do_exit,
};

static void exit_cmd_init(void)
{
	register_command(&cmd_exit);
}

static void exit_cmd_exit(void)
{
	unregister_command(&cmd_exit);
}

command_initcall(exit_cmd_init);
command_exitcall(exit_cmd_exit);
