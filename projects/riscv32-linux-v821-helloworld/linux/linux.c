#include <mm_common.h>
#include <mpi_sys.h>
#include <utils/plat_log.h>
#include <linux/linux.h>

static struct termios tconfig;

static void allwinner_mpp_init(void)
{
	MPP_SYS_CONF_S sysConf;
	memset(&sysConf, 0, sizeof(MPP_SYS_CONF_S));
	sysConf.nAlignWidth = 32;
	AW_MPI_SYS_SetConf(&sysConf);
	AW_MPI_SYS_Init();
	extern int MPP_GLOBAL_LOG_LEVEL;
	MPP_GLOBAL_LOG_LEVEL = OPTION_LOG_LEVEL_CLOSE;
}

static void allwinner_mpp_exit(void)
{
	//AW_MPI_SYS_Exit();
}

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
	allwinner_mpp_init();
}

void linux_exit(void)
{
	allwinner_mpp_exit();
	fflush(stdout);
	tcsetattr(0, TCSANOW, &tconfig);
	exit(0);
}
