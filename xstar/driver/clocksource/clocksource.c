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

/*
 * Dummy clocksource, 10us - 100KHZ
 */
static uint64_t __cs_dummy_read(struct clocksource_t * cs)
{
	static volatile uint64_t __cs_dummy_cycle = 0;
	return __cs_dummy_cycle++;
}

static struct clocksource_t __cs_dummy = {
	.keeper = {
		.interval = 35184372083832,
		.last = 0,
		.nsec = 0,
		.seqlock = { 0 },
	},
	.name = "cs-dummy",
	.mask = CLOCKSOURCE_MASK(64),
	.mult = 2621440000,
	.shift = 18,
	.read = __cs_dummy_read,
};
static struct clocksource_t * __clocksource = &__cs_dummy;
static struct mutex_t __clocksource_lock;

static ssize_t clocksource_read_mult(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clocksource_t * cs = (struct clocksource_t *)kobj->priv;
	return xos_sprintf(buf, "%u", cs->mult);
}

static ssize_t clocksource_read_shift(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clocksource_t * cs = (struct clocksource_t *)kobj->priv;
	return xos_sprintf(buf, "%u", cs->shift);
}

static ssize_t clocksource_read_period(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clocksource_t * cs = (struct clocksource_t *)kobj->priv;
	uint64_t period = ((uint64_t)cs->mult) >> cs->shift;
	return xos_sprintf(buf, "%Lu.%09Lu", period / 1000000000ULL, period % 1000000000ULL);
}

static ssize_t clocksource_read_deferment(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clocksource_t * cs = (struct clocksource_t *)kobj->priv;
	uint64_t max = clocksource_deferment(cs);
	return xos_sprintf(buf, "%Lu.%09Lu", max / 1000000000ULL, max % 1000000000ULL);
}

static ssize_t clocksource_read_cycle(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clocksource_t * cs = (struct clocksource_t *)kobj->priv;
	return xos_sprintf(buf, "%Lu", clocksource_cycle(cs));
}

static ssize_t clocksource_read_time(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clocksource_t * cs = (struct clocksource_t *)kobj->priv;
	uint64_t cycle = clocksource_cycle(cs);
	uint64_t time = clocksource_delta2ns(cs, cycle);
	return xos_sprintf(buf, "%Lu.%09Lu", time / 1000000000ULL, time % 1000000000ULL);
}

static ssize_t clocksource_read_ktime(struct kobj_t * kobj, void * buf, size_t size)
{
	struct clocksource_t * cs = (struct clocksource_t *)kobj->priv;
	uint64_t ktime = ktime_to_ns(clocksource_keeper_read(cs));
	return xos_sprintf(buf, "%Lu.%09Lu", ktime / 1000000000ULL, ktime % 1000000000ULL);
}

static int clocksource_keeper_timer_function(struct timer_t * timer, void * data)
{
	struct clocksource_t * cs = (struct clocksource_t *)(data);
	uint64_t now, delta, offset;

	++cs->keeper.seqlock.seq;
	now = clocksource_cycle(cs);
	delta = clocksource_delta(cs, cs->keeper.last, now);
	offset = clocksource_delta2ns(cs, delta);
	cs->keeper.nsec += offset;
	cs->keeper.last = now;
	cs->keeper.seqlock.seq++;

	timer_forward(timer, ns_to_ktime(cs->keeper.interval));
	return 1;
}

struct clocksource_t * search_clocksource(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_CLOCKSOURCE);
	if(!dev)
		return NULL;
	return (struct clocksource_t *)dev->priv;
}

struct clocksource_t * search_first_clocksource(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_CLOCKSOURCE);
	if(!dev)
		return NULL;
	return (struct clocksource_t *)dev->priv;
}

struct device_t * register_clocksource(struct clocksource_t * cs, struct driver_t * drv)
{
	struct device_t * dev;

	if(!cs || !cs->name || !cs->read)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	cs->keeper.interval = clocksource_deferment(cs) >> 1;
	cs->keeper.last = clocksource_cycle(cs);
	cs->keeper.nsec = 0;
	cs->keeper.seqlock.seq = 0;
	timer_init(&cs->keeper.timer, clocksource_keeper_timer_function, cs);

	dev->name = xos_strdup(cs->name);
	dev->type = DEVICE_TYPE_CLOCKSOURCE;
	dev->driver = drv;
	dev->priv = cs;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "mult", clocksource_read_mult, NULL, cs);
	kobj_add_regular(dev->kobj, "shift", clocksource_read_shift, NULL, cs);
	kobj_add_regular(dev->kobj, "period", clocksource_read_period, NULL, cs);
	kobj_add_regular(dev->kobj, "deferment", clocksource_read_deferment, NULL, cs);
	kobj_add_regular(dev->kobj, "cycle", clocksource_read_cycle, NULL, cs);
	kobj_add_regular(dev->kobj, "time", clocksource_read_time, NULL, cs);
	kobj_add_regular(dev->kobj, "ktime", clocksource_read_ktime, NULL, cs);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	if(__clocksource == &__cs_dummy)
	{
		xos_mutex_lock(&__clocksource_lock);
		__clocksource = cs;
		xos_mutex_unlock(&__clocksource_lock);
	}
	timer_start(&cs->keeper.timer, ns_to_ktime(cs->keeper.interval));
	return dev;
}

void unregister_clocksource(struct clocksource_t * cs)
{
	struct device_t * dev;
	struct clocksource_t * c;

	if(cs && cs->name && cs->read)
	{
		dev = search_device(cs->name, DEVICE_TYPE_CLOCKSOURCE);
		if(dev && unregister_device(dev))
		{
			timer_cancel(&cs->keeper.timer);
			if(__clocksource == cs)
			{
				if(!(c = search_first_clocksource()))
					c = &__cs_dummy;
				xos_mutex_lock(&__clocksource_lock);
				__clocksource = c;
				xos_mutex_unlock(&__clocksource_lock);
			}
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

ktime_t clocksource_ktime_get(struct clocksource_t * cs)
{
	if(cs)
		return clocksource_keeper_read(cs);
	return ns_to_ktime(0);
}

ktime_t ktime_get(void)
{
	return clocksource_keeper_read(__clocksource);
}

static void clocksource_pure_init(void)
{
	xos_mutex_init(&__clocksource_lock);
}

static void clocksource_pure_exit(void)
{
	xos_mutex_exit(&__clocksource_lock);
}

pure_initcall(clocksource_pure_init);
pure_exitcall(clocksource_pure_exit);
