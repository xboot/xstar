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

#include <driver/oximeter/oximeter.h>

static ssize_t oximeter_read_spo2(struct kobj_t * kobj, void * buf, size_t size)
{
	struct oximeter_t * oximeter = (struct oximeter_t *)kobj->priv;
	int spo2 = 0;
	oximeter_get(oximeter, &spo2, NULL);
	return xos_sprintf(buf, "%d%%", spo2);
}

static ssize_t oximeter_read_heartrate(struct kobj_t * kobj, void * buf, size_t size)
{
	struct oximeter_t * oximeter = (struct oximeter_t *)kobj->priv;
	int heartrate = 0;
	oximeter_get(oximeter, NULL, &heartrate);
	return xos_sprintf(buf, "%d", heartrate);
}

struct oximeter_t * search_oximeter(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_OXIMETER);
	if(!dev)
		return NULL;
	return (struct oximeter_t *)dev->priv;
}

struct oximeter_t * search_first_oximeter(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_OXIMETER);
	if(!dev)
		return NULL;
	return (struct oximeter_t *)dev->priv;
}

struct device_t * register_oximeter(struct oximeter_t * oximeter, struct driver_t * drv)
{
	struct device_t * dev;

	if(!oximeter || !oximeter->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(oximeter->name);
	dev->type = DEVICE_TYPE_OXIMETER;
	dev->driver = drv;
	dev->priv = oximeter;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "spo2", oximeter_read_spo2, NULL, oximeter);
	kobj_add_regular(dev->kobj, "heartrate", oximeter_read_heartrate, NULL, oximeter);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_oximeter(struct oximeter_t * oximeter)
{
	struct device_t * dev;

	if(oximeter && oximeter->name)
	{
		dev = search_device(oximeter->name, DEVICE_TYPE_OXIMETER);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int oximeter_get(struct oximeter_t * oximeter, int * spo2, int * heartrate)
{
	if(oximeter && oximeter->get)
		return oximeter->get(oximeter, spo2, heartrate);
	return 0;
}
