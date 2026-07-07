#include <linux/linux.h>

int linux_i2c_exist(const char * dev)
{
	struct stat st;

	if((stat(dev, &st) == 0) && S_ISCHR(st.st_mode) && (access(dev, R_OK | W_OK) == 0))
		return 1;
	return 0;
}

int linux_i2c_open(const char * dev)
{
	int fd;

	fd = open(dev, O_RDWR);
	if(fd > 0)
	{
		unsigned long funcs = 0;
		if((ioctl(fd, I2C_FUNCS, &funcs) >= 0) && (funcs & I2C_FUNC_I2C))
			return fd;
		close(fd);
	}
	return 0;
}

void linux_i2c_close(int fd)
{
	if(fd > 0)
		close(fd);
}

int linux_i2c_xfer(int fd, struct linux_i2c_msg_t * msgs, int num)
{
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg msg[num];

	for(int i = 0; i < num; i++)
	{
		msg[i].addr = msgs[i].addr;
		msg[i].flags = msgs[i].flags;
		msg[i].len = msgs[i].len;
		msg[i].buf = msgs[i].buf;
	}
	packets.msgs = msg;
	packets.nmsgs = num;
	return (ioctl(fd, I2C_RDWR, &packets) == num) ? num : 0;
}
