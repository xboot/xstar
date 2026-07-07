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

#include <kernel/time/timer.h>
#include <driver/clockevent/clockevent.h>

/*
 * Dummy clockevent, 1us - 1MHZ
 */
static void __ce_dummy_handler(struct clockevent_t * ce, void * data)
{
}

static int __ce_dummy_next(struct clockevent_t * ce, uint64_t evt)
{
	return TRUE;
}

static struct clockevent_t __ce_dummy = {
	.name = "ce-dummy",
	.mult = 4294967,
	.shift = 32,
	.min_delta_ns = 1000,
	.max_delta_ns = 4294967591000,
	.data = NULL,
	.handler = __ce_dummy_handler,
	.next = __ce_dummy_next,
};
static struct clockevent_t * __clockevent = &__ce_dummy;
static struct mutex_t __clockevent_lock;

static ssize_t clockevent_read_mult(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clockevent_t * ce = (struct clockevent_t *)kobj->priv;
	return xos_sprintf(buf, "%u", ce->mult);
}

static ssize_t clockevent_read_shift(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clockevent_t * ce = (struct clockevent_t *)kobj->priv;
	return xos_sprintf(buf, "%u", ce->shift);
}

static ssize_t clockevent_read_frequency(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clockevent_t * ce = (struct clockevent_t *)kobj->priv;
	uint64_t freq = ((uint64_t)1000000000ULL * ce->mult) >> ce->shift;
	return xos_sprintf(buf, "%Lu", freq);
}

static ssize_t clockevent_read_min_delta(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clockevent_t * ce = (struct clockevent_t *)kobj->priv;
	return xos_sprintf(buf, "%llu.%09llu", ce->min_delta_ns / 1000000000ULL, ce->min_delta_ns % 1000000000ULL);
}

static ssize_t clockevent_read_max_delta(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clockevent_t * ce = (struct clockevent_t *)kobj->priv;
	return xos_sprintf(buf, "%llu.%09llu", ce->max_delta_ns / 1000000000ULL, ce->max_delta_ns % 1000000000ULL);
}

struct clockevent_t * search_clockevent(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_CLOCKEVENT);
	if(!dev)
		return NULL;
	return (struct clockevent_t *)dev->priv;
}

struct clockevent_t * search_first_clockevent(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_CLOCKEVENT);
	if(!dev)
		return NULL;
	return (struct clockevent_t *)dev->priv;
}

struct device_t * register_clockevent(struct clockevent_t * ce, struct driver_t * drv)
{
	struct device_t * dev;

	if(!ce || !ce->name || !ce->next)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	ce->data = NULL;
	ce->handler = __ce_dummy_handler;

	dev->name = xos_strdup(ce->name);
	dev->type = DEVICE_TYPE_CLOCKEVENT;
	dev->driver = drv;
	dev->priv = ce;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "mult", clockevent_read_mult, NULL, ce);
	kobj_add_regular(dev->kobj, "shift", clockevent_read_shift, NULL, ce);
	kobj_add_regular(dev->kobj, "frequency", clockevent_read_frequency, NULL, ce);
	kobj_add_regular(dev->kobj, "min_delta", clockevent_read_min_delta, NULL, ce);
	kobj_add_regular(dev->kobj, "max_delta", clockevent_read_max_delta, NULL, ce);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	if(__clockevent == &__ce_dummy)
	{
		xos_mutex_lock(&__clockevent_lock);
		__clockevent = ce;
		timer_bind_clockevent(__clockevent);
		xos_mutex_unlock(&__clockevent_lock);
	}
	return dev;
}

void unregister_clockevent(struct clockevent_t * ce)
{
	struct device_t * dev;
	struct clockevent_t * c;

	if(ce && ce->name && ce->next)
	{
		dev = search_device(ce->name, DEVICE_TYPE_CLOCKEVENT);
		if(dev && unregister_device(dev))
		{
			if(__clockevent == ce)
			{
				if(!(c = search_first_clockevent()))
					c = &__ce_dummy;
				xos_mutex_lock(&__clockevent_lock);
				__clockevent = c;
				timer_bind_clockevent(__clockevent);
				xos_mutex_unlock(&__clockevent_lock);
			}
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int clockevent_set_event_handler(struct clockevent_t * ce, void (*handler)(struct clockevent_t *, void *), void * data)
{
	if(!ce)
		return FALSE;
	ce->data = data;
	ce->handler = handler ? handler : __ce_dummy_handler;
	return TRUE;
}

int clockevent_set_event_next(struct clockevent_t * ce, ktime_t now, ktime_t expires)
{
	uint64_t delta;

	if(!ce)
		return FALSE;

	if(ktime_before(expires, now))
		return FALSE;

	delta = ktime_to_ns(ktime_sub(expires, now));
	if(delta > ce->max_delta_ns)
		delta = ce->max_delta_ns;
	if(delta < ce->min_delta_ns)
		delta = ce->min_delta_ns;
	return ce->next(ce, ((uint64_t)delta * ce->mult) >> ce->shift);
}

static void clockevent_pure_init(void)
{
	xos_mutex_init(&__clockevent_lock);
}

static void clockevent_pure_exit(void)
{
	xos_mutex_exit(&__clockevent_lock);
}

pure_initcall(clockevent_pure_init);
pure_exitcall(clockevent_pure_exit);
