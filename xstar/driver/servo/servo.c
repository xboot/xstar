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

#include <driver/servo/servo.h>

static ssize_t servo_write_enable(struct kobj_t * kobj, void * buf, size_t size)
{
	struct servo_t * m = (struct servo_t *)kobj->priv;
	servo_enable(m);
	return size;
}

static ssize_t servo_write_disable(struct kobj_t * kobj, void * buf, size_t size)
{
	struct servo_t * m = (struct servo_t *)kobj->priv;
	servo_disable(m);
	return size;
}

static ssize_t servo_read_range(struct kobj_t * kobj, void * buf, size_t size)
{
	struct servo_t * m = (struct servo_t *)kobj->priv;
	return xos_sprintf(buf, "%d", servo_get_range(m));
}

static ssize_t servo_read_angle(struct kobj_t * kobj, void * buf, size_t size)
{
	struct servo_t * m = (struct servo_t *)kobj->priv;
	return xos_sprintf(buf, "%d", servo_get_angle(m));
}

static ssize_t servo_write_angle(struct kobj_t * kobj, void * buf, size_t size)
{
	struct servo_t * m = (struct servo_t *)kobj->priv;
	servo_set_angle(m, xos_strtol(buf, NULL, 0));
	return size;
}

struct servo_t * search_servo(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_SERVO);
	if(!dev)
		return NULL;
	return (struct servo_t *)dev->priv;
}

struct device_t * register_servo(struct servo_t * m, struct driver_t * drv)
{
	struct device_t * dev;

	if(!m || !m->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(m->name);
	dev->type = DEVICE_TYPE_SERVO;
	dev->driver = drv;
	dev->priv = m;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "enable", NULL, servo_write_enable, m);
	kobj_add_regular(dev->kobj, "disable", NULL, servo_write_disable, m);
	kobj_add_regular(dev->kobj, "range", servo_read_range, NULL, m);
	kobj_add_regular(dev->kobj, "angle", servo_read_angle, servo_write_angle, m);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_servo(struct servo_t * m)
{
	struct device_t * dev;

	if(m && m->name)
	{
		dev = search_device(m->name, DEVICE_TYPE_SERVO);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

void servo_enable(struct servo_t * m)
{
	if(m && m->enable)
		m->enable(m);
}

void servo_disable(struct servo_t * m)
{
	if(m && m->disable)
		m->disable(m);
}

int servo_get_range(struct servo_t * m)
{
	if(m && m->range)
		return m->range(m);
	return 0;
}

int servo_get_angle(struct servo_t * m)
{
	if(m && m->get)
		return m->get(m);
	return 0;
}

void servo_set_angle(struct servo_t * m, int angle)
{
	if(m && m->set)
	{
		angle = XCLAMP(angle, 0, servo_get_range(m));
		m->set(m, angle);
	}
}
