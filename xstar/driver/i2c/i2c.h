#ifndef __XSTAR_DRIVER_I2C_H__
#define __XSTAR_DRIVER_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

enum {
	I2C_MODE_TEN			= 0x0010,
	I2C_MODE_RD				= 0x0001,
	I2C_MODE_STOP			= 0x8000,
	I2C_MODE_NOSTART		= 0x4000,
	I2C_MODE_REV_DIR_ADDR	= 0x2000,
	I2C_MODE_IGNORE_NAK		= 0x1000,
	I2C_MODE_NO_RD_ACK		= 0x0800,
	I2C_MODE_RECV_LEN		= 0x0400,
};

struct i2c_msg_t {
	int addr;
	int flags;
	int len;
	void * buf;
};

struct i2c_t {
	/* The i2c bus name */
	char * name;

	/* Master xfer */
	int (*xfer)(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num);

	/* Private data */
	void * priv;
};

struct i2c_device_t {
	struct i2c_t * i2c;
	int addr;
	int flags;
};

struct i2c_t * search_i2c(const char * name);
struct device_t * register_i2c(struct i2c_t * i2c, struct driver_t * drv);
void unregister_i2c(struct i2c_t * i2c);

struct i2c_device_t * i2c_device_alloc(const char * i2cbus, int addr, int flags);
void i2c_device_free(struct i2c_device_t * client);
int i2c_transfer(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num);
int i2c_master_send(const struct i2c_device_t * dev, void * buf, int count);
int i2c_master_recv(const struct i2c_device_t * dev, void * buf, int count);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_I2C_H__ */
