#include <linux/linux.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    example\r\n");
}

static int do_example(int argc, char ** argv)
{
	shell_printf("hello, example!\r\n");
	return 0;
}

static struct command_t cmd_example = {
	.name	= "example",
	.desc	= "an example command for user directory",
	.usage	= usage,
	.exec	= do_example,
};

static void example_cmd_init(void)
{
	register_command(&cmd_example);
}

static void example_cmd_exit(void)
{
	unregister_command(&cmd_example);
}

command_initcall(example_cmd_init);
command_exitcall(example_cmd_exit);
