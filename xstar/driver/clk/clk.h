#ifndef __XSTAR_DRIVER_CLK_H__
#define __XSTAR_DRIVER_CLK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct clk_t {
	char * name;
	int count;

	void (*set_parent)(struct clk_t * clk, const char * pname);
	const char * (*get_parent)(struct clk_t * clk);
	void (*set_enable)(struct clk_t * clk, int enable);
	int (*get_enable)(struct clk_t * clk);
	void (*set_rate)(struct clk_t * clk, uint64_t prate, uint64_t rate);
	uint64_t (*get_rate)(struct clk_t * clk, uint64_t prate);
	void * priv;
};

struct clk_t * search_clk(const char * name);
struct device_t * register_clk(struct clk_t * clk, struct driver_t * drv);
void unregister_clk(struct clk_t * clk);

void clk_set_parent(const char * name, const char * pname);
const char * clk_get_parent(const char * name);
void clk_enable(const char * name);
void clk_disable(const char * name);
int clk_status(const char * name);
void clk_set_rate(const char * name, uint64_t rate);
uint64_t clk_get_rate(const char * name);

struct clocks_t {
	int n;
	struct clk_t * clk[0];
};

struct clocks_t * clocks_alloc(struct dtnode_t * n, const char * name);
void clocks_free(struct clocks_t * clks);
void clocks_enable(struct clocks_t * clks);
void clocks_disable(struct clocks_t * clks);
const char * clocks_get(struct clocks_t * clks, int idx);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_CLK_H__ */
