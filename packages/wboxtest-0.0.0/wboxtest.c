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

#include <wboxtest.h>

static struct hlist_head_t __wboxtest_hash[257];
static struct mutex_t __wboxtest_lock;

static struct hlist_head_t * wboxtest_hash(const char * group)
{
	return &__wboxtest_hash[shash(group) % ARRAY_SIZE(__wboxtest_hash)];
}

static struct kobj_t * search_class_wboxtest_group_kobj(const char * group)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	struct kobj_t * kwboxtest = kobj_search_directory_with_create(kclass, "wboxtest");
	return kobj_search_directory_with_create(kwboxtest, group);
}

static void wboxtest_run(struct wboxtest_t * wbt, int count)
{
	void * data;
	int i;

	if(wbt && (count > 0))
	{
		wboxtest_printf("\033[43;37m[%s]-[%s]\033[0m\r\n", wbt->group, wbt->name);
		data = wbt->setup(wbt);
		for(i = 0; i < count; i++)
		{
			if(count > 1)
				wboxtest_printf("\033[44;37m[%d]\033[0m\r\n", i);
			wbt->run(wbt, data);
		}
		wbt->clean(wbt, data);
	}
}

static ssize_t wboxtest_write_test(struct kobj_t * kobj, void * buf, size_t size)
{
	struct wboxtest_t * wbt = (struct wboxtest_t *)kobj->priv;
	int count = xos_strtol(buf, NULL, 0);
	wboxtest_run(wbt, count);
	return size;
}

struct wboxtest_t * search_wboxtest(const char * group, const char * name)
{
	struct wboxtest_t * pos;
	struct hlist_node_t * n;

	if(!group || !name)
		return NULL;

	hlist_for_each_entry_safe(pos, n, wboxtest_hash(group), node)
	{
		if((xos_strcmp(pos->group, group) == 0) && (xos_strcmp(pos->name, name) == 0))
			return pos;
	}
	return NULL;
}

int register_wboxtest(struct wboxtest_t * wbt)
{
	if(!wbt || !wbt->group || !wbt->name)
		return FALSE;

	if(!wbt->setup || !wbt->clean || !wbt->run)
		return FALSE;

	if(search_wboxtest(wbt->group, wbt->name))
		return FALSE;

	wbt->kobj = kobj_alloc_directory(wbt->name);
	kobj_add_regular(wbt->kobj, "test", NULL, wboxtest_write_test, wbt);
	kobj_add(search_class_wboxtest_group_kobj(wbt->group), wbt->kobj);

	xos_mutex_lock(&__wboxtest_lock);
	init_hlist_node(&wbt->node);
	hlist_add_head(&wbt->node, wboxtest_hash(wbt->group));
	xos_mutex_unlock(&__wboxtest_lock);

	return TRUE;
}

int unregister_wboxtest(struct wboxtest_t * wbt)
{
	if(!wbt || !wbt->group || !wbt->name)
		return FALSE;

	if(hlist_unhashed(&wbt->node))
		return FALSE;

	xos_mutex_lock(&__wboxtest_lock);
	hlist_del(&wbt->node);
	xos_mutex_unlock(&__wboxtest_lock);
	kobj_remove_self(wbt->kobj);

	return TRUE;
}

void wboxtest_run_group_name(const char * group, const char * name, int count)
{
	if(group && name && (count > 0))
		wboxtest_run(search_wboxtest(group, name), count);
}

void wboxtest_run_group(const char * group, int count)
{
	struct wboxtest_t * pos;
	struct hlist_node_t * n;
	struct slist_t * sl, * e;

	if(group && (count > 0))
	{
		sl = slist_alloc();
		hlist_for_each_entry_safe(pos, n, wboxtest_hash(group), node)
		{
			if((xos_strcmp(pos->group, group) == 0))
				slist_add(sl, pos, "%s", pos->name);
		}
		slist_sort(sl);
		slist_for_each_entry(e, sl)
		{
			wboxtest_run((struct wboxtest_t *)e->priv, count);
		}
		slist_free(sl);
	}
}

void wboxtest_run_all(int count)
{
	struct wboxtest_t * pos;
	struct hlist_node_t * n;
	struct slist_t * sl, * e;
	int i;

	if(count > 0)
	{
		sl = slist_alloc();
		for(i = 0; i < ARRAY_SIZE(__wboxtest_hash); i++)
		{
			hlist_for_each_entry_safe(pos, n, &__wboxtest_hash[i], node)
			{
				slist_add(sl, pos, "%s%s", pos->group, pos->name);
			}
		}
		slist_sort(sl);
		slist_for_each_entry(e, sl)
		{
			wboxtest_run((struct wboxtest_t *)e->priv, count);
		}
		slist_free(sl);
	}
}

void wboxtest_list(void)
{
	struct wboxtest_t * pos;
	struct hlist_node_t * n;
	struct slist_t * sl, * e;
	int i;

	sl = slist_alloc();
	for(i = 0; i < ARRAY_SIZE(__wboxtest_hash); i++)
	{
		hlist_for_each_entry_safe(pos, n, &__wboxtest_hash[i], node)
		{
			slist_add(sl, pos, "%s%s", pos->group, pos->name);
		}
	}
	slist_sort(sl);
	slist_for_each_entry(e, sl)
	{
		pos = (struct wboxtest_t *)e->priv;
		wboxtest_printf("[%s]-[%s]\r\n", pos->group, pos->name);
	}
	slist_free(sl);
}

int wboxtest_random_int(int lower, int upper)
{
	return xos_random_int(lower, upper);
}

float wboxtest_random_float(float lower, float upper)
{
	return xos_random_float(lower, upper);
}

char * wboxtest_random_string(char * buf, int len)
{
	char c;
	int i;

	if(buf && len > 0)
	{
		for(i = 0; i < len;)
		{
			c = xos_rand() & 0x7f;
			if(xos_isupper(c) || xos_islower(c))
			{
				buf[i] = c;
				i++;
			}
		}
		buf[len] = '\0';
	}
	return buf;
}

char * wboxtest_random_buffer(char * buf, int len)
{
	int i;

	if(buf && len > 0)
	{
		for(i = 0; i < len; i++)
			buf[i] = xos_rand() & 0xff;
	}
	return buf;
}

int wboxtest_printf(const char * fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = shell_vprintf(fmt, ap);
	va_end(ap);
	return len;
}

void wboxtest_assert(int cond, char * expr, const char * file, int line)
{
	int len = wboxtest_printf(" [%s:%d] { %s }", file, line, expr);
	wboxtest_printf("%*s\r\n", 80 + 12 - 6 - len, cond ? "\033[42;37m[OKAY]\033[0m" : "\033[41;37m[FAIL]\033[0m");
}

static void wboxtest_pure_init(void)
{
	for(int i = 0; i < ARRAY_SIZE(__wboxtest_hash); i++)
		init_hlist_head(&__wboxtest_hash[i]);
	xos_mutex_init(&__wboxtest_lock);
}

static void wboxtest_pure_exit(void)
{
	xos_mutex_exit(&__wboxtest_lock);
}

pure_initcall(wboxtest_pure_init);
pure_exitcall(wboxtest_pure_exit);
