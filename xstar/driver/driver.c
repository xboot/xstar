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

#include <kernel/core/logger.h>
#include <driver/driver.h>

static struct hlist_head_t __driver_hash[521];
static struct mutex_t __driver_lock;

static struct hlist_head_t * driver_hash(const char * name)
{
	return &__driver_hash[shash(name) % ARRAY_SIZE(__driver_hash)];
}

static struct kobj_t * search_class_driver_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "driver");
}

static ssize_t driver_write_probe(struct kobj_t * kobj, void * buf, size_t size)
{
	struct driver_t * drv = (struct driver_t *)kobj->priv;
	struct dtnode_t n;
	struct json_value_t * v;

	if(buf && (size > 0))
	{
		v = json_parse(buf, size, 0);
		if(v && (v->type == JSON_OBJECT))
		{
			for(int i = 0; i < v->u.object.length; i++)
			{
				char * p = (char *)(v->u.object.values[i].name);
				if(p)
				{
					char * name = p;
					char * id = NULL;
					char * addr = NULL;
					while(*p)
					{
						if(*p == ':')
						{
							*p = '\0';
							id = p + 1;
							if((*id == ':') || (*id == '@'))
								id = NULL;
						}
						else if(*p == '@')
						{
							*p = '\0';
							addr = p + 1;
							if((*addr == ':') || (*addr == '@'))
								addr = NULL;
						}
						p++;
					}
					n.name = (name && (*name != '\0')) ? name : "";
					n.id = (id && (*id != '\0')) ? xos_strtol(id, NULL, 0) : 0;
					n.addr = (addr && (*addr != '\0')) ? xos_strtoull(addr, NULL, 0) : 0;
					n.value = (struct json_value_t *)(v->u.object.values[i].value);

					if(xos_strcmp(dt_read_string(&n, "status", "okay"), "disabled") != 0)
					{
						if(xos_strcmp(drv->name, n.name) == 0)
							drv->probe(drv, &n);
					}
				}
			}
		}
		json_free(v);
	}
	return size;
}

struct driver_t * search_driver(const char * name)
{
	struct driver_t * pos;
	struct hlist_node_t * n;

	if(!name)
		return NULL;

	hlist_for_each_entry_safe(pos, n, driver_hash(name), node)
	{
		if(xos_strcmp(pos->name, name) == 0)
			return pos;
	}
	return NULL;
}

int register_driver(struct driver_t * drv)
{
	if(!drv || !drv->name)
		return FALSE;

	if(!drv->probe || !drv->remove)
		return FALSE;

	if(!drv->suspend || !drv->resume)
		return FALSE;

	if(search_driver(drv->name))
		return FALSE;

	drv->kobj = kobj_alloc_directory(drv->name);
	kobj_add_regular(drv->kobj, "probe", NULL, driver_write_probe, drv);
	kobj_add(search_class_driver_kobj(), drv->kobj);

	xos_mutex_lock(&__driver_lock);
	init_hlist_node(&drv->node);
	hlist_add_head(&drv->node, driver_hash(drv->name));
	xos_mutex_unlock(&__driver_lock);

	return TRUE;
}

int unregister_driver(struct driver_t * drv)
{
	if(!drv || !drv->name)
		return FALSE;

	if(hlist_unhashed(&drv->node))
		return FALSE;

	xos_mutex_lock(&__driver_lock);
	hlist_del(&drv->node);
	xos_mutex_unlock(&__driver_lock);
	kobj_remove_self(drv->kobj);

	return TRUE;
}

void probe_device(const char * json, int length)
{
	struct driver_t * drv;
	struct device_t * dev;
	struct dtnode_t n;
	struct json_value_t * v;
	char errbuf[256];

	if(json && (length > 0))
	{
		v = json_parse(json, length, errbuf);
		if(v && (v->type == JSON_OBJECT))
		{
			for(int i = 0; i < v->u.object.length; i++)
			{
				char *  p = (char *)(v->u.object.values[i].name);
				if(p)
				{
					char * name = p;
					char * id = NULL;
					char * addr = NULL;
					while(*p)
					{
						if(*p == ':')
						{
							*p = '\0';
							id = p + 1;
							if((*id == ':') || (*id == '@'))
								id = NULL;
						}
						else if(*p == '@')
						{
							*p = '\0';
							addr = p + 1;
							if((*addr == ':') || (*addr == '@'))
								addr = NULL;
						}
						p++;
					}
					n.name = (name && (*name != '\0')) ? name : "";
					n.id = (id && (*id != '\0')) ? xos_strtol(id, NULL, 0) : 0;
					n.addr = (addr && (*addr != '\0')) ? xos_strtoull(addr, NULL, 0) : 0;
					n.value = (struct json_value_t *)(v->u.object.values[i].value);

					if(xos_strcmp(dt_read_string(&n, "status", "okay"), "disabled") != 0)
					{
						drv = search_driver(n.name);
						if(drv && (dev = drv->probe(drv, &n)))
							LOG("Probe device '%s' with %s\r\n", dev->name, drv->name);
						else
							LOG("Fail to probe device with %s\r\n", n.name);
					}
				}
			}
		}
		else
		{
			LOG("[Json]-%s\r\n", errbuf);
		}
		json_free(v);
	}
}

void remove_device(struct device_t * dev)
{
	if(dev && dev->driver && dev->driver->remove)
		dev->driver->remove(dev);
}

static void driver_pure_init(void)
{
	for(int i = 0; i < ARRAY_SIZE(__driver_hash); i++)
		init_hlist_head(&__driver_hash[i]);
	xos_mutex_init(&__driver_lock);
}

static void driver_pure_exit(void)
{
	xos_mutex_exit(&__driver_lock);
}

pure_initcall(driver_pure_init);
pure_exitcall(driver_pure_exit);
