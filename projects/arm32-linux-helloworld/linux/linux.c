#include <linux/linux.h>

static struct termios tconfig;

void linux_init(void)
{
	struct termios ta;

	tcgetattr(0, &tconfig);
	ta = tconfig;
	ta.c_lflag &= (~ICANON & ~ECHO & ~ISIG);
	ta.c_iflag &= (~IXON & ~ICRNL);
	ta.c_oflag |= (ONLCR);
	ta.c_cc[VMIN] = 1;
	ta.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &ta);
	linux_file_cwd();
}

void linux_exit(void)
{
	fflush(stdout);
	tcsetattr(0, TCSANOW, &tconfig);
	exit(0);
}
