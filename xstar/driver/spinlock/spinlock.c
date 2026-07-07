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

#include <driver/spinlock/spinlock.h>

static void __sh_init(struct spinlock_hdl_t * hdl, struct spinlock_t * lock)
{
	lock->lock = 0;
}

static int __sh_trylock(struct spinlock_hdl_t * hdl, struct spinlock_t * lock)
{
	volatile int flag = 0;

	if(lock->lock == 0)
	{
		lock->lock = 1;
		flag = 1;
	}
	return flag;
}

static void __sh_lock(struct spinlock_hdl_t * hdl, struct spinlock_t * lock)
{
	while(lock->lock != 0);
}

static void __sh_unlock(struct spinlock_hdl_t * hdl, struct spinlock_t * lock)
{
	lock->lock = 0;
}

static struct spinlock_hdl_t __sh_dummy = {
	.name = "sh-dummy",
	.init = __sh_init,
	.trylock = __sh_trylock,
	.lock = __sh_lock,
	.unlock = __sh_unlock,
	.priv = NULL,
};
static struct spinlock_hdl_t * __spinlock_hdl = &__sh_dummy;
static struct mutex_t __spinlock_lock;

struct spinlock_hdl_t * search_spinlock(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_SPINLOCK);
	if(!dev)
		return NULL;
	return (struct spinlock_hdl_t *)dev->priv;
}

struct spinlock_hdl_t * search_first_spinlock(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_SPINLOCK);
	if(!dev)
		return NULL;
	return (struct spinlock_hdl_t *)dev->priv;
}

struct device_t * register_spinlock(struct spinlock_hdl_t * hdl, struct driver_t * drv)
{
	struct device_t * dev;

	if(!hdl || !hdl->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(hdl->name);
	dev->type = DEVICE_TYPE_SPINLOCK;
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
	if(__spinlock_hdl == &__sh_dummy)
	{
		xos_mutex_lock(&__spinlock_lock);
		__spinlock_hdl = hdl;
		xos_mutex_unlock(&__spinlock_lock);
	}
	return dev;
}

void unregister_spinlock(struct spinlock_hdl_t * hdl)
{
	struct device_t * dev;
	struct spinlock_hdl_t * h;

	if(hdl && hdl->name)
	{
		dev = search_device(hdl->name, DEVICE_TYPE_SPINLOCK);
		if(dev && unregister_device(dev))
		{
			if(__spinlock_hdl == hdl)
			{
				if(!(h = search_first_spinlock()))
					h = &__sh_dummy;
				xos_mutex_lock(&__spinlock_lock);
				__spinlock_hdl = h;
				xos_mutex_unlock(&__spinlock_lock);
			}
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

void spinlock_init(struct spinlock_t * lock)
{
	__spinlock_hdl->init(__spinlock_hdl, lock);
}

int spinlock_trylock(struct spinlock_t * lock)
{
	return __spinlock_hdl->trylock(__spinlock_hdl, lock);
}

void spinlock_lock(struct spinlock_t * lock)
{
	__spinlock_hdl->lock(__spinlock_hdl, lock);
}

void spinlock_unlock(struct spinlock_t * lock)
{
	__spinlock_hdl->unlock(__spinlock_hdl, lock);
}

static void spinlock_pure_init(void)
{
	xos_mutex_init(&__spinlock_lock);
}

static void spinlock_pure_exit(void)
{
	xos_mutex_exit(&__spinlock_lock);
}

pure_initcall(spinlock_pure_init);
pure_exitcall(spinlock_pure_exit);
