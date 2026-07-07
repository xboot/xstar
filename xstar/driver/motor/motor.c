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

#include <driver/motor/motor.h>

static ssize_t motor_write_enable(struct kobj_t * kobj, void * buf, size_t size)
{
	struct motor_t * m = (struct motor_t *)kobj->priv;
	motor_enable(m);
	return size;
}

static ssize_t motor_write_disable(struct kobj_t * kobj, void * buf, size_t size)
{
	struct motor_t * m = (struct motor_t *)kobj->priv;
	motor_disable(m);
	return size;
}

static ssize_t motor_write_speed(struct kobj_t * kobj, void * buf, size_t size)
{
	struct motor_t * m = (struct motor_t *)kobj->priv;
	motor_set_speed(m, xos_strtol(buf, NULL, 0));
	return size;
}

struct motor_t * search_motor(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_MOTOR);
	if(!dev)
		return NULL;
	return (struct motor_t *)dev->priv;
}

struct motor_t * search_first_motor(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_MOTOR);
	if(!dev)
		return NULL;
	return (struct motor_t *)dev->priv;
}

struct device_t * register_motor(struct motor_t * m, struct driver_t * drv)
{
	struct device_t * dev;

	if(!m || !m->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(m->name);
	dev->type = DEVICE_TYPE_MOTOR;
	dev->driver = drv;
	dev->priv = m;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "enable", NULL, motor_write_enable, m);
	kobj_add_regular(dev->kobj, "disable", NULL, motor_write_disable, m);
	kobj_add_regular(dev->kobj, "speed", NULL, motor_write_speed, m);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_motor(struct motor_t * m)
{
	struct device_t * dev;

	if(m && m->name)
	{
		dev = search_device(m->name, DEVICE_TYPE_MOTOR);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

void motor_enable(struct motor_t * m)
{
	if(m && m->enable)
		m->enable(m);
}

void motor_disable(struct motor_t * m)
{
	if(m && m->disable)
		m->disable(m);
}

void motor_set_speed(struct motor_t * m, int speed)
{
	if(m && m->set)
		m->set(m, speed);
}

/*
 * motor async
 */
struct motor_async_ctx_t * motor_async_ctx_alloc(const char * name)
{
	struct motor_t * motor = name ? search_motor(name) : search_first_motor();

	if(motor)
	{
		struct thworker_t * worker = thworker_alloc(motor->name);
		if(worker)
		{
			struct motor_async_ctx_t * ctx = xos_mem_malloc(sizeof(struct motor_async_ctx_t));
			if(!ctx)
			{
				thworker_free(worker);
				return NULL;
			}
			ctx->motor = motor;
			ctx->worker = worker;
			return ctx;
		}
	}
	return NULL;
}

void motor_async_ctx_free(struct motor_async_ctx_t * ctx)
{
	if(ctx)
	{
		thworker_wait(ctx->worker);
		thworker_free(ctx->worker);
		xos_mem_free(ctx);
	}
}

static void clrcb(void (*func)(void *), void * data)
{
	if(data)
		xos_mem_free(data);
}

void motor_async_ctx_clear(struct motor_async_ctx_t * ctx)
{
	if(ctx)
	{
		thworker_clear(ctx->worker, clrcb);
		motor_set_speed(ctx->motor, 0);
	}
}

struct motor_async_enable_pdat_t {
	struct motor_t * motor;
};

static void motor_async_enable_func(void * data)
{
	struct motor_async_enable_pdat_t * pdat = (struct motor_async_enable_pdat_t *)data;

	if(pdat)
	{
		motor_enable(pdat->motor);
		xos_mem_free(pdat);
	}
}

void motor_async_enable(struct motor_async_ctx_t * ctx)
{
	if(ctx)
	{
		struct motor_async_enable_pdat_t * pdat = xos_mem_malloc(sizeof(struct motor_async_enable_pdat_t));
		if(pdat)
		{
			pdat->motor = ctx->motor;
			thworker_submit(ctx->worker, motor_async_enable_func, pdat);
		}
	}
}

struct motor_async_disable_pdat_t {
	struct motor_t * motor;
};

static void motor_async_disable_func(void * data)
{
	struct motor_async_disable_pdat_t * pdat = (struct motor_async_disable_pdat_t *)data;

	if(pdat)
	{
		motor_disable(pdat->motor);
		xos_mem_free(pdat);
	}
}

void motor_async_disable(struct motor_async_ctx_t * ctx)
{
	if(ctx)
	{
		struct motor_async_disable_pdat_t * pdat = xos_mem_malloc(sizeof(struct motor_async_disable_pdat_t));
		if(pdat)
		{
			pdat->motor = ctx->motor;
			thworker_submit(ctx->worker, motor_async_disable_func, pdat);
		}
	}
}

struct motor_async_play_pdat_t {
	struct motor_t * motor;
	int speed;
	int millisecond;
};

static void motor_async_play_func(void * data)
{
	struct motor_async_play_pdat_t * pdat = (struct motor_async_play_pdat_t *)data;

	if(pdat)
	{
		motor_set_speed(pdat->motor, pdat->speed);
		if(pdat->millisecond > 0)
			xos_thread_msleep(pdat->millisecond);
		xos_mem_free(pdat);
	}
}

void motor_async_play(struct motor_async_ctx_t * ctx, int speed, int millisecond)
{
	if(ctx)
	{
		struct motor_async_play_pdat_t * pdat = xos_mem_malloc(sizeof(struct motor_async_play_pdat_t));
		if(pdat)
		{
			pdat->motor = ctx->motor;
			pdat->speed = speed;
			pdat->millisecond = millisecond;
			thworker_submit(ctx->worker, motor_async_play_func, pdat);
		}
	}
}
