#include <linux/linux.h>

struct gpiochip_info {
	char name[32];
	char label[32];
	uint32_t lines;
};

struct gpioline_info {
	uint32_t line_offset;
	uint32_t flags;
	char name[32];
	char consumer[32];
};

struct gpiohandle_request {
	uint32_t lineoffsets[64];
	uint32_t flags;
	uint8_t default_values[64];
	char consumer_label[32];
	uint32_t lines;
	int fd;
};

struct gpiohandle_data {
	uint8_t values[64];
};

#define GPIOLINE_FLAG_KERNEL				(1 << 0)
#define GPIOLINE_FLAG_IS_OUT				(1 << 1)
#define GPIOLINE_FLAG_ACTIVE_LOW			(1 << 2)
#define GPIOLINE_FLAG_OPEN_DRAIN			(1 << 3)
#define GPIOLINE_FLAG_OPEN_SOURCE			(1 << 4)

#define GPIOHANDLE_REQUEST_INPUT			(1 << 0)
#define GPIOHANDLE_REQUEST_OUTPUT			(1 << 1)
#define GPIOHANDLE_REQUEST_ACTIVE_LOW		(1 << 2)
#define GPIOHANDLE_REQUEST_OPEN_DRAIN		(1 << 3)
#define GPIOHANDLE_REQUEST_OPEN_SOURCE		(1 << 4)

#define GPIO_GET_CHIPINFO_IOCTL				_IOR(0xB4, 0x01, struct gpiochip_info)
#define GPIO_GET_LINEINFO_IOCTL				_IOWR(0xB4, 0x02, struct gpioline_info)
#define GPIO_GET_LINEHANDLE_IOCTL			_IOWR(0xB4, 0x03, struct gpiohandle_request)
#define GPIOHANDLE_GET_LINE_VALUES_IOCTL	_IOWR(0xB4, 0x08, struct gpiohandle_data)
#define GPIOHANDLE_SET_LINE_VALUES_IOCTL	_IOWR(0xB4, 0x09, struct gpiohandle_data)

int linux_gpio_v1_open(const char * path, int * ngpio)
{
	int fd = open(path, O_RDWR | O_CLOEXEC);
	if(fd > 0)
	{
		struct gpiochip_info info;
		if(ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info) == 0)
		{
			if(ngpio)
				*ngpio = info.lines;
			return fd;
		}
	}
	return 0;
}

void linux_gpio_v1_close(int fd)
{
	if(fd > 0)
		close(fd);
}

int linux_gpio_v1_open_handle(int fd, int offset, int dir, int val)
{
	struct gpiohandle_request req;
	memset(&req, 0, sizeof(req));
	req.lineoffsets[0] = offset;
	req.flags = dir ? GPIOHANDLE_REQUEST_OUTPUT : GPIOHANDLE_REQUEST_INPUT;
	req.default_values[0] = val;
	req.lines = 1;
	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == 0)
		return req.fd;
	return 0;
}

void linux_gpio_v1_close_handle(int handle)
{
	if(handle > 0)
		close(handle);
}

int linux_gpio_v1_info(int fd, int offset, int * dir)
{
	struct gpioline_info info;

	memset(&info, 0, sizeof(info));
	info.line_offset = offset;
	if(ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &info) == 0)
	{
		if(dir)
			*dir = (info.flags & GPIOLINE_FLAG_IS_OUT) ? 1 : 0;
		if(!(info.flags & GPIOLINE_FLAG_KERNEL))
			return 1;
	}
	return 0;
}

int linux_gpio_v1_get_value(int handle, int offset)
{
	struct gpiohandle_data gha;

	if(ioctl(handle, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &gha) == 0)
		return gha.values[0] ? 1 : 0;
	return 0;
}

void linux_gpio_v1_set_value(int handle, int offset, int value)
{
	struct gpiohandle_data gha;

	gha.values[0] = value ? 1 : 0;
	ioctl(handle, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gha);
}
