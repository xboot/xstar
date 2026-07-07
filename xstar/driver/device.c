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

#include <kernel/core/psub.h>
#include <driver/device.h>

struct list_head_t __device_list;
struct list_head_t __device_head[DEVICE_TYPE_MAX_COUNT];
static struct hlist_head_t __device_hash[521];
static struct mutex_t __device_lock;

static struct hlist_head_t * device_hash(const char * name)
{
	return &__device_hash[shash(name) % ARRAY_SIZE(__device_hash)];
}

static struct kobj_t * search_device_kobj(struct device_t * dev)
{
	struct kobj_t * kdevice;
	char * name;

	if(!dev || !dev->kobj)
		return NULL;

	kdevice = kobj_search_directory_with_create(kobj_get_root(), "device");
	if(!kdevice)
		return NULL;

	switch(dev->type)
	{
	case DEVICE_TYPE_ADC:
		name = "adc";
		break;
	case DEVICE_TYPE_ATNET:
		name = "atnet";
		break;
	case DEVICE_TYPE_ATOMIC:
		name = "atomic";
		break;
	case DEVICE_TYPE_AUDIOCAPTURE:
		name = "audiocapture";
		break;
	case DEVICE_TYPE_AUDIOPLAYBACK:
		name = "audioplayback";
		break;
	case DEVICE_TYPE_BATTERY:
		name = "battery";
		break;
	case DEVICE_TYPE_BLOCK:
		name = "block";
		break;
	case DEVICE_TYPE_BUZZER:
		name = "buzzer";
		break;
	case DEVICE_TYPE_CAMERA:
		name = "camera";
		break;
	case DEVICE_TYPE_CLK:
		name = "clk";
		break;
	case DEVICE_TYPE_CLOCKEVENT:
		name = "clockevent";
		break;
	case DEVICE_TYPE_CLOCKSOURCE:
		name = "clocksource";
		break;
	case DEVICE_TYPE_COMPASS:
		name = "compass";
		break;
	case DEVICE_TYPE_CONSOLE:
		name = "console";
		break;
	case DEVICE_TYPE_DAC:
		name = "dac";
		break;
	case DEVICE_TYPE_DMACHIP:
		name = "dmachip";
		break;
	case DEVICE_TYPE_FRAMEBUFFER:
		name = "framebuffer";
		break;
	case DEVICE_TYPE_G2D:
		name = "g2d";
		break;
	case DEVICE_TYPE_GMETER:
		name = "gmeter";
		break;
	case DEVICE_TYPE_GNSS:
		name = "gnss";
		break;
	case DEVICE_TYPE_GPIOCHIP:
		name = "gpiochip";
		break;
	case DEVICE_TYPE_GYROSCOPE:
		name = "gyroscope";
		break;
	case DEVICE_TYPE_HYGROMETER:
		name = "hygrometer";
		break;
	case DEVICE_TYPE_I2C:
		name = "i2c";
		break;
	case DEVICE_TYPE_INPUT:
		name = "input";
		break;
	case DEVICE_TYPE_IRQCHIP:
		name = "irqchip";
		break;
	case DEVICE_TYPE_LED:
		name = "led";
		break;
	case DEVICE_TYPE_LEDSTRIP:
		name = "ledstrip";
		break;
	case DEVICE_TYPE_LEDTRIGGER:
		name = "ledtrigger";
		break;
	case DEVICE_TYPE_LIGHT:
		name = "light";
		break;
	case DEVICE_TYPE_LIMITER:
		name = "limiter";
		break;
	case DEVICE_TYPE_MOTOR:
		name = "motor";
		break;
	case DEVICE_TYPE_NET:
		name = "net";
		break;
	case DEVICE_TYPE_NVMEM:
		name = "nvmem";
		break;
	case DEVICE_TYPE_OXIMETER:
		name = "oximeter";
		break;
	case DEVICE_TYPE_PRESSURE:
		name = "pressure";
		break;
	case DEVICE_TYPE_PRINTER:
		name = "printer";
		break;
	case DEVICE_TYPE_PROXIMITY:
		name = "proximity";
		break;
	case DEVICE_TYPE_PWM:
		name = "pwm";
		break;
	case DEVICE_TYPE_REGULATOR:
		name = "regulator";
		break;
	case DEVICE_TYPE_RESETCHIP:
		name = "resetchip";
		break;
	case DEVICE_TYPE_RNG:
		name = "rng";
		break;
	case DEVICE_TYPE_RTC:
		name = "rtc";
		break;
	case DEVICE_TYPE_SDHCI:
		name = "sdhci";
		break;
	case DEVICE_TYPE_SERVO:
		name = "servo";
		break;
	case DEVICE_TYPE_SPI:
		name = "spi";
		break;
	case DEVICE_TYPE_SPINLOCK:
		name = "spinlock";
		break;
	case DEVICE_TYPE_STEPPER:
		name = "stepper";
		break;
	case DEVICE_TYPE_THERMOMETER:
		name = "thermometer";
		break;
	case DEVICE_TYPE_UART:
		name = "uart";
		break;
	case DEVICE_TYPE_VIBRATOR:
		name = "vibrator";
		break;
	case DEVICE_TYPE_WATCHDOG:
		name = "watchdog";
		break;
	default:
		return NULL;
	}

	return kobj_search_directory_with_create(kdevice, (const char *)name);
}

static ssize_t device_write_suspend(struct kobj_t * kobj, void * buf, size_t size)
{
	struct device_t * dev = (struct device_t *)kobj->priv;

	if(xos_strncmp(buf, dev->name, size) == 0)
		suspend_device(dev);
	return size;
}

static ssize_t device_write_resume(struct kobj_t * kobj, void * buf, size_t size)
{
	struct device_t * dev = (struct device_t *)kobj->priv;

	if(xos_strncmp(buf, dev->name, size) == 0)
		resume_device(dev);
	return size;
}

static int device_exist(const char * name)
{
	struct device_t * pos;
	struct hlist_node_t * n;

	hlist_for_each_entry_safe(pos, n, device_hash(name), node)
	{
		if(xos_strcmp(pos->name, name) == 0)
			return TRUE;
	}
	return FALSE;
}

char * alloc_device_name(const char * name, int id)
{
	char buf[256];

	if(id < 0)
		id = 0;
	do {
		xos_snprintf(buf, sizeof(buf), "%s.%d", name, id++);
	} while(device_exist(buf));

	return xos_strdup(buf);
}

void free_device_name(char * name)
{
	if(name)
		xos_mem_free(name);
}

struct device_t * search_device(const char * name, enum device_type_t type)
{
	struct device_t * pos;
	struct hlist_node_t * n;

	if(!name)
		return NULL;

	hlist_for_each_entry_safe(pos, n, device_hash(name), node)
	{
		if((pos->type == type) && (xos_strcmp(pos->name, name) == 0))
			return pos;
	}
	return NULL;
}

struct device_t * search_first_device(enum device_type_t type)
{
	if((type < 0) || (type >= ARRAY_SIZE(__device_head)))
		return NULL;
	return (struct device_t *)list_first_entry_or_null(&__device_head[type], struct device_t, head);
}

int register_device(struct device_t * dev)
{
	if(!dev || !dev->name)
		return FALSE;

	if((dev->type < 0) || (dev->type >= ARRAY_SIZE(__device_head)))
		return FALSE;

	if(device_exist(dev->name))
		return FALSE;

	kobj_add_regular(dev->kobj, "suspend", NULL, device_write_suspend, dev);
	kobj_add_regular(dev->kobj, "resume", NULL, device_write_resume, dev);
	kobj_add(search_device_kobj(dev), dev->kobj);

	xos_mutex_lock(&__device_lock);
	init_list_head(&dev->list);
	list_add_tail(&dev->list, &__device_list);
	init_list_head(&dev->head);
	list_add_tail(&dev->head, &__device_head[dev->type]);
	init_hlist_node(&dev->node);
	hlist_add_head(&dev->node, device_hash(dev->name));
	xos_mutex_unlock(&__device_lock);
	psub_publish("device.add", dev);

	return TRUE;
}

int unregister_device(struct device_t * dev)
{
	if(!dev || !dev->name)
		return FALSE;

	if((dev->type < 0) || (dev->type >= ARRAY_SIZE(__device_head)))
		return FALSE;

	if(hlist_unhashed(&dev->node))
		return FALSE;

	psub_publish("device.remove", dev);
	xos_mutex_lock(&__device_lock);
	list_del(&dev->list);
	list_del(&dev->head);
	hlist_del(&dev->node);
	xos_mutex_unlock(&__device_lock);
	kobj_remove(search_device_kobj(dev), dev->kobj);

	return TRUE;
}

void suspend_device(struct device_t * dev)
{
	if(dev)
	{
		psub_publish("device.suspend", dev);
		if(dev->driver && dev->driver->suspend)
			dev->driver->suspend(dev);
	}
}

void resume_device(struct device_t * dev)
{
	if(dev)
	{
		if(dev->driver && dev->driver->resume)
			dev->driver->resume(dev);
		psub_publish("device.resume", dev);
	}
}

static void device_pure_init(void)
{
	init_list_head(&__device_list);
	for(int i = 0; i < ARRAY_SIZE(__device_head); i++)
		init_list_head(&__device_head[i]);
	for(int i = 0; i < ARRAY_SIZE(__device_hash); i++)
		init_hlist_head(&__device_hash[i]);
	xos_mutex_init(&__device_lock);
}

static void device_pure_exit(void)
{
	xos_mutex_exit(&__device_lock);
}

pure_initcall(device_pure_init);
pure_exitcall(device_pure_exit);
