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

#include <driver/atomic/atomic.h>

static void __ah_set(struct atomic_hdl_t * hdl, struct atomic_t * a, int v)
{
	do {
		a->counter = v;
	} while(0);
}

static int __ah_get(struct atomic_hdl_t * hdl, struct atomic_t * a)
{
	return a->counter;
}

static int __ah_add(struct atomic_hdl_t * hdl, struct atomic_t * a, int v)
{
	a->counter += v;
	return a->counter;
}

static int __ah_sub(struct atomic_hdl_t * hdl, struct atomic_t * a, int v)
{
	a->counter -= v;
	return a->counter;
}

static int __ah_cas(struct atomic_hdl_t * hdl, struct atomic_t * a, int o, int n)
{
	volatile int v = a->counter;
	if(v == o)
		a->counter = n;
	return v;
}

static struct atomic_hdl_t __ah_dummy = {
	.name = "ah-dummy",
	.set = __ah_set,
	.get = __ah_get,
	.add = __ah_add,
	.sub = __ah_sub,
	.cas = __ah_cas,
	.priv = NULL,
};
static struct atomic_hdl_t * __atomic_hdl = &__ah_dummy;
static struct mutex_t __atomic_lock;

struct atomic_hdl_t * search_atomic(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_ATOMIC);
	if(!dev)
		return NULL;
	return (struct atomic_hdl_t *)dev->priv;
}

struct atomic_hdl_t * search_first_atomic(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_ATOMIC);
	if(!dev)
		return NULL;
	return (struct atomic_hdl_t *)dev->priv;
}

struct device_t * register_atomic(struct atomic_hdl_t * hdl, struct driver_t * drv)
{
	struct device_t * dev;

	if(!hdl || !hdl->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(hdl->name);
	dev->type = DEVICE_TYPE_ATOMIC;
	dev->driver = drv;
	dev->priv = hdl;
	dev->kobj = kobj_alloc_directory(dev->name);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	if(__atomic_hdl == &__ah_dummy)
	{
		xos_mutex_lock(&__atomic_lock);
		__atomic_hdl = hdl;
		xos_mutex_unlock(&__atomic_lock);
	}
	return dev;
}

void unregister_atomic(struct atomic_hdl_t * hdl)
{
	struct device_t * dev;
	struct atomic_hdl_t * h;

	if(hdl && hdl->name)
	{
		dev = search_device(hdl->name, DEVICE_TYPE_ATOMIC);
		if(dev && unregister_device(dev))
		{
			if(__atomic_hdl == hdl)
			{
				if(!(h = search_first_atomic()))
					h = &__ah_dummy;
				xos_mutex_lock(&__atomic_lock);
				__atomic_hdl = h;
				xos_mutex_unlock(&__atomic_lock);
			}
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

void atomic_set(struct atomic_t * a, int v)
{
	__atomic_hdl->set(__atomic_hdl, a, v);
}

int atomic_get(struct atomic_t * a)
{
	return __atomic_hdl->get(__atomic_hdl, a);
}

int atomic_add(struct atomic_t * a, int v)
{
	return __atomic_hdl->add(__atomic_hdl, a, v);
}

int atomic_sub(struct atomic_t * a, int v)
{
	return __atomic_hdl->sub(__atomic_hdl, a, v);
}

int atomic_cas(struct atomic_t * a, int o, int n)
{
	return __atomic_hdl->cas(__atomic_hdl, a, o, n);
}

void atomic_pure_init(void)
{
	xos_mutex_init(&__atomic_lock);
}

static void atomic_pure_exit(void)
{
	xos_mutex_exit(&__atomic_lock);
}

pure_initcall(atomic_pure_init);
pure_exitcall(atomic_pure_exit);
