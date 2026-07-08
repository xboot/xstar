#include <linux/linux.h>

void linux_pm_shutdown(void)
{
	sync();
	reboot(RB_POWER_OFF);
}

void linux_pm_reboot(void)
{
	sync();
	reboot(RB_AUTOBOOT);
}

void linux_pm_standby(void)
{
	sync();
	linux_sysfs_write_string("/sys/power/state", "mem");
}
