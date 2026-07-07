/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <driver/clocksource/clocksource.h>
#include <driver/uart/uart.h>
#include <driver/atnet/atnet.h>

/*
 * ML307 - Cat.1 communication module
 *
 * Required properties:
 * - uart-bus: uart bus name which the device is attached to
 *
 * Optional properties:
 * - baud-rates: communication baud rate (default: 115200)
 * - data-bits: number of data bits per character
 * - parity-bits: parity bit configuration (0: none, 1: odd, 2: even)
 * - stop-bits: number of stop bits (1 or 2)
 *
 * Example:
 *   "atnet-ml307": {
 *       "uart-bus": "uart-linux.0",
 *       "baud-rates": 115200,
 *       "data-bits": 8,
 *       "parity-bits": 0,
 *       "stop-bits": 1
 *   }
 */

struct atnet_ml307_pdata_t {
	struct mutex_t lock;
	struct uart_t * uart;
};

int ml307_at_request(struct atnet_ml307_pdata_t * pdat, const char * cmd, char * resp, int respsz, int timeout)
{
	uint8_t buf[256];
	int len = 0;
	int ok = 0;
	int n;

	xos_mutex_lock(&pdat->lock);
	{
		/*
		 * drain any data
		 */
		while(uart_read(pdat->uart, buf, sizeof(buf)) > 0);

		/*
		 * request
		 */
		if(cmd && ((n = strlen(cmd)) > 0))
			uart_write(pdat->uart, (const uint8_t *)cmd, n);

		/*
		 * response
		 */
		ktime_t time = ktime_add_ms(ktime_get(), timeout);
		do {
			while((n = uart_read(pdat->uart, buf, sizeof(buf))) > 0)
			{
				for(int i = 0; i < n; i++)
				{
					char c = buf[i];
					if(resp && (len < respsz - 1))
						resp[len++] = c;
				}
			}
		} while(!ok && ktime_before(ktime_get(), time));

		if(resp && (len < respsz - 1))
			resp[len] = '\0';
	}
	xos_mutex_unlock(&pdat->lock);
	return ok ? len : 0;
}

static int ml307_detect(struct uart_t * uart, int baud, int data, int parity, int stop)
{
	return 0;
}

static struct device_t * atnet_ml307_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct atnet_ml307_pdata_t * pdat;
	struct atnet_t * net;
	struct device_t * dev;
	struct uart_t * uart = search_uart(dt_read_string(n, "uart-bus", NULL));
	int baud = dt_read_int(n, "baud-rates", 115200);
	int data = dt_read_int(n, "data-bits", 8);
	int parity = dt_read_int(n, "parity-bits", 0);
	int stop = dt_read_int(n, "stop-bits", 1);

	if(ml307_detect(uart, baud, data, parity, stop))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct atnet_ml307_pdata_t));
	if(!pdat)
		return NULL;

	net = xos_mem_malloc(sizeof(struct atnet_t));
	if(!net)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct atnet_ml307_pdata_t));
	xos_mutex_init(&pdat->lock);
	pdat->uart = uart;

	net->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	net->priv = pdat;

	if(!(dev = register_atnet(net, drv)))
	{
		xos_mutex_exit(&pdat->lock);
		free_device_name(net->name);
		xos_mem_free(net->priv);
		xos_mem_free(net);
		return NULL;
	}
	return dev;
}

static void atnet_ml307_remove(struct device_t * dev)
{
	struct atnet_t * net = (struct atnet_t *)dev->priv;
	struct atnet_ml307_pdata_t * pdat = (struct atnet_ml307_pdata_t *)net->priv;

	if(net)
	{
		xos_mutex_exit(&pdat->lock);
		unregister_atnet(net);
		free_device_name(net->name);
		xos_mem_free(net->priv);
		xos_mem_free(net);
	}
}

static void atnet_ml307_suspend(struct device_t * dev)
{
}

static void atnet_ml307_resume(struct device_t * dev)
{
}

static struct driver_t atnet_ml307 = {
	.name		= "atnet-ml307",
	.probe		= atnet_ml307_probe,
	.remove		= atnet_ml307_remove,
	.suspend	= atnet_ml307_suspend,
	.resume		= atnet_ml307_resume,
};

static void atnet_ml307_driver_init(void)
{
	register_driver(&atnet_ml307);
}

static void atnet_ml307_driver_exit(void)
{
	unregister_driver(&atnet_ml307);
}

driver_initcall(atnet_ml307_driver_init);
driver_exitcall(atnet_ml307_driver_exit);
