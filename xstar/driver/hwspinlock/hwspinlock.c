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

#include <driver/hwspinlock/hwspinlock.h>

static ssize_t hwspinlock_read_base(struct kobj_t * kobj, void * buf, size_t size)
{
	struct hwspinlock_t * hsl = (struct hwspinlock_t *)kobj->priv;
	return xos_sprintf(buf, "%d", hsl->base);
}

static ssize_t hwspinlock_read_nlock(struct kobj_t * kobj, void * buf, size_t size)
{
	struct hwspinlock_t * hsl = (struct hwspinlock_t *)kobj->priv;
	return xos_sprintf(buf, "%d", hsl->nlock);
}

struct hwspinlock_t * search_hwspinlock(int lock)
{
	struct device_t * pos, * n;
	struct hwspinlock_t * hsl;

	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_HWSPINLOCK], head)
	{
		hsl = (struct hwspinlock_t *)(pos->priv);
		if((lock >= hsl->base) && (lock < (hsl->base + hsl->nlock)))
			return hsl;
	}
	return NULL;
}

struct device_t * register_hwspinlock(struct hwspinlock_t * hsl, struct driver_t * drv)
{
	struct device_t * dev;

	if(!hsl || !hsl->name)
		return NULL;

	if((hsl->base < 0) || (hsl->nlock <= 0))
		return NULL;

	if(!hsl->trylock || !hsl->lock || !hsl->unlock)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(hsl->name);
	dev->type = DEVICE_TYPE_HWSPINLOCK;
	dev->driver = drv;
	dev->priv = hsl;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "base", hwspinlock_read_base, NULL, hsl);
	kobj_add_regular(dev->kobj, "nlock", hwspinlock_read_nlock, NULL, hsl);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_hwspinlock(struct hwspinlock_t * hsl)
{
	struct device_t * dev;

	if(hsl && hsl->name)
	{
		dev = search_device(hsl->name, DEVICE_TYPE_HWSPINLOCK);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int hwspinlock_is_valid(int lock)
{
	return search_hwspinlock(lock) ? 1 : 0;
}

int hwspinlock_trylock(int lock)
{
	struct hwspinlock_t * hsl = search_hwspinlock(lock);

	if(hsl)
		return hsl->trylock(hsl, lock - hsl->base);
	return 0;
}

void hwspinlock_lock(int lock)
{
	struct hwspinlock_t * hsl = search_hwspinlock(lock);

	if(hsl)
		hsl->lock(hsl, lock - hsl->base);
}

void hwspinlock_unlock(int lock)
{
	struct hwspinlock_t * hsl = search_hwspinlock(lock);

	if(hsl)
		hsl->unlock(hsl, lock - hsl->base);
}

struct hwspinlock_desc_t * hwspinlock_desc_alloc(int lock)
{
	struct hwspinlock_t * hsl = search_hwspinlock(lock);

	if(hsl)
	{
		struct hwspinlock_desc_t * desc = xos_mem_malloc(sizeof(struct hwspinlock_desc_t));
		if(desc)
		{
			desc->hsl = hsl;
			desc->offset = lock - hsl->base;
			return desc;
		}
	}
	return NULL;
}

void hwspinlock_desc_free(struct hwspinlock_desc_t * desc)
{
	if(desc)
		xos_mem_free(desc);
}
