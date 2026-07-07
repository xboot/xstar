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

#include <driver/vibrator/vibrator.h>

static ssize_t vibrator_read_state(struct kobj_t * kobj, void * buf, size_t size)
{
	struct vibrator_t * vib = (struct vibrator_t *)kobj->priv;
	return xos_sprintf(buf, "%d", vibrator_get_state(vib));
}

static ssize_t vibrator_write_state(struct kobj_t * kobj, void * buf, size_t size)
{
	struct vibrator_t * vib = (struct vibrator_t *)kobj->priv;
	vibrator_set_state(vib, xos_strtol(buf, NULL, 0));
	return size;
}

struct vibrator_t * search_vibrator(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_VIBRATOR);
	if(!dev)
		return NULL;
	return (struct vibrator_t *)dev->priv;
}

struct vibrator_t * search_first_vibrator(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_VIBRATOR);
	if(!dev)
		return NULL;
	return (struct vibrator_t *)dev->priv;
}

struct device_t * register_vibrator(struct vibrator_t * vib, struct driver_t * drv)
{
	struct device_t * dev;

	if(!vib || !vib->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(vib->name);
	dev->type = DEVICE_TYPE_VIBRATOR;
	dev->driver = drv;
	dev->priv = vib;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "state", vibrator_read_state, vibrator_write_state, vib);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_vibrator(struct vibrator_t * vib)
{
	struct device_t * dev;

	if(vib && vib->name)
	{
		dev = search_device(vib->name, DEVICE_TYPE_VIBRATOR);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

void vibrator_set_state(struct vibrator_t * vib, int state)
{
	if(vib && vib->set)
		vib->set(vib, (state > 0) ? 1 : 0);
}

int vibrator_get_state(struct vibrator_t * vib)
{
	if(vib && vib->get)
		return vib->get(vib);
	return 0;
}

/*
 * vibrator async
 */
struct vibrator_async_ctx_t * vibrator_async_ctx_alloc(const char * name)
{
	struct vibrator_t * vib = name ? search_vibrator(name) : search_first_vibrator();

	if(vib)
	{
		struct thworker_t * worker = thworker_alloc(vib->name);
		if(worker)
		{
			struct vibrator_async_ctx_t * ctx = xos_mem_malloc(sizeof(struct vibrator_async_ctx_t));
			if(!ctx)
			{
				thworker_free(worker);
				return NULL;
			}
			ctx->vibrator = vib;
			ctx->worker = worker;
			return ctx;
		}
	}
	return NULL;
}

void vibrator_async_ctx_free(struct vibrator_async_ctx_t * ctx)
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

void vibrator_async_ctx_clear(struct vibrator_async_ctx_t * ctx)
{
	if(ctx)
	{
		thworker_clear(ctx->worker, clrcb);
		vibrator_set_state(ctx->vibrator, 0);
	}
}

struct vibrator_async_vibrate_pdat_t {
	struct vibrator_t * vibrator;
	int state;
	int millisecond;
};

static void vibrator_async_vibrate_func(void * data)
{
	struct vibrator_async_vibrate_pdat_t * pdat = (struct vibrator_async_vibrate_pdat_t *)data;

	if(pdat)
	{
		vibrator_set_state(pdat->vibrator, pdat->state);
		if(pdat->millisecond > 0)
			xos_thread_msleep(pdat->millisecond);
		xos_mem_free(pdat);
	}
}

void vibrator_async_vibrate(struct vibrator_async_ctx_t * ctx, int state, int millisecond)
{
	if(ctx)
	{
		struct vibrator_async_vibrate_pdat_t * pdat = xos_mem_malloc(sizeof(struct vibrator_async_vibrate_pdat_t));
		if(pdat)
		{
			pdat->vibrator = ctx->vibrator;
			pdat->state = state;
			pdat->millisecond = millisecond;
			thworker_submit(ctx->worker, vibrator_async_vibrate_func, pdat);
		}
	}
}

static const char * morse_code(char c)
{
	switch(c)
	{
	case '0':
		return "-----";
	case '1':
		return ".----";
	case '2':
		return "..---";
	case '3':
		return "...--";
	case '4':
		return "....-";
	case '5':
		return ".....";
	case '6':
		return "-....";
	case '7':
		return "--...";
	case '8':
		return "---..";
	case '9':
		return "----.";

	case 'a':
	case 'A':
		return ".-";
	case 'b':
	case 'B':
		return "-...";
	case 'c':
	case 'C':
		return "-.-.";
	case 'd':
	case 'D':
		return "-..";
	case 'e':
	case 'E':
		return ".";
	case 'f':
	case 'F':
		return "..-.";
	case 'g':
	case 'G':
		return "--.";
	case 'h':
	case 'H':
		return "....";
	case 'i':
	case 'I':
		return "..";
	case 'j':
	case 'J':
		return ".---";
	case 'k':
	case 'K':
		return "-.-";
	case 'l':
	case 'L':
		return ".-..";
	case 'm':
	case 'M':
		return "--";
	case 'n':
	case 'N':
		return "-.";
	case 'o':
	case 'O':
		return "---";
	case 'p':
	case 'P':
		return ".--.";
	case 'q':
	case 'Q':
		return "--.-";
	case 'r':
	case 'R':
		return ".-.";
	case 's':
	case 'S':
		return "...";
	case 't':
	case 'T':
		return "-";
	case 'u':
	case 'U':
		return "..-";
	case 'v':
	case 'V':
		return "...-";
	case 'w':
	case 'W':
		return ".--";
	case 'x':
	case 'X':
		return "-..-";
	case 'y':
	case 'Y':
		return "-.--";
	case 'z':
	case 'Z':
		return "--..";

	case '.':
		return ".-.-.-";
	case ',':
		return "--..--";
	case '?':
		return "..--..";
	case '\'':
		return ".----.";
	case '!':
		return "-.-.--";
	case '/':
		return "-..-.";
	case '(':
		return "-.--.-";
	case ')':
		return "-.--.-";
	case '&':
		return ".-...";
	case ':':
		return "---...";
	case ';':
		return "-.-.-.";
	case '=':
		return "-...-";
	case '+':
		return ".-.-.";
	case '-':
		return "-....-";
	case '_':
		return "..--.-";
	case '\"':
		return ".-..-.";
	case '$':
		return "...-..-";
	case '@':
		return ".--.-.";

	default:
		break;
	}
	return "";
}

void vibrator_async_play(struct vibrator_async_ctx_t * ctx, const char * morse)
{
	char * p = (char *)morse;
	if(p)
	{
		while(*p)
		{
			if(xos_isspace(*p))
			{
				vibrator_async_vibrate(ctx, 0, 100 * 7);
				continue;
			}
			char * q = (char *)morse_code(*p);
			while(*q)
			{
				if(*q == '.')
					vibrator_async_vibrate(ctx, 1, 100 * 1);
				else if(*q == '-')
					vibrator_async_vibrate(ctx, 1, 100 * 3);

				vibrator_async_vibrate(ctx, 0, 100);
				q++;
			}
			vibrator_async_vibrate(ctx, 0, 100 * 3);
			p++;
		}
	}
}
