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

#include <kernel/audio/effect.h>

static struct list_head_t __audio_effect_list = {
	.next = &__audio_effect_list,
	.prev = &__audio_effect_list,
};
static struct mutex_t __audio_effect_lock;

static struct audio_effect_t * search_audio_effect(const char * name)
{
	struct audio_effect_t * pos, * n;

	if(!name)
		return NULL;

	list_for_each_entry_safe(pos, n, &__audio_effect_list, list)
	{
		if(xos_strcmp(pos->name, name) == 0)
			return pos;
	}
	return NULL;
}

int register_audio_effect(struct audio_effect_t * ae)
{
	if(!ae || !ae->name || !ae->create || !ae->setup || !ae->prepare || !ae->process || !ae->destroy)
		return FALSE;

	if(search_audio_effect(ae->name))
		return FALSE;

	xos_mutex_lock(&__audio_effect_lock);
	list_add_tail(&ae->list, &__audio_effect_list);
	xos_mutex_unlock(&__audio_effect_lock);
	return TRUE;
}

int unregister_audio_effect(struct audio_effect_t * ae)
{
	if(!ae || !ae->name)
		return FALSE;

	xos_mutex_lock(&__audio_effect_lock);
	list_del(&ae->list);
	xos_mutex_unlock(&__audio_effect_lock);
	return TRUE;
}

static inline int audio_effect_node_length(const char * json, int len)
{
	int n = 0;

	if(json && (len > 0))
	{
		struct json_value_t * v = json_parse(json, len, NULL);
		if(v && (v->type == JSON_OBJECT))
		{
			for(int i = 0; i < v->u.object.length; i++)
			{
				if(search_audio_effect(v->u.object.values[i].name))
					n++;
			}
		}
		json_free(v);
	}
	return n;
}

struct audio_filter_t * audio_filter_alloc(const char * json, int len)
{
	struct audio_filter_t * filter;
	int nlen;

	nlen = audio_effect_node_length(json, len);
	if(nlen <= 0)
		return NULL;

	filter = xos_mem_malloc(sizeof(struct audio_filter_t));
	if(!filter)
		return NULL;

	filter->nodes = xos_mem_malloc(sizeof(struct audio_effect_node_t) * nlen);
	if(!filter->nodes)
	{
		xos_mem_free(filter);
		return NULL;
	}
	filter->nlen = 0;

	if(json && (len > 0))
	{
		struct json_value_t * v = json_parse(json, len, NULL);
		if(v && (v->type == JSON_OBJECT))
		{
			for(int i = 0; i < v->u.object.length; i++)
			{
				struct json_value_t * o = v->u.object.values[i].value;
				struct audio_effect_t * ae = search_audio_effect(v->u.object.values[i].name);
				if(ae)
				{
					struct audio_effect_node_t * node = &filter->nodes[filter->nlen];
					node->bypass = 0;
					node->ae = ae;
					node->ctx = node->ae->create();
					node->ae->setup(node->ctx, o);
					filter->nlen++;
				}
			}
		}
		json_free(v);
	}

	return filter;
}

void audio_filter_free(struct audio_filter_t * filter)
{
	if(filter)
	{
		for(int i = 0; i < filter->nlen; i++)
		{
			struct audio_effect_node_t * node = &filter->nodes[i];
			if(node->ae)
				node->ae->destroy(node->ctx);
		}
		xos_mem_free(filter->nodes);
		xos_mem_free(filter);
	}
}

void audio_filter_bypass(struct audio_filter_t * filter, int id, int bypass)
{
	if(filter)
	{
		if((id >= 0) && (id < filter->nlen))
		{
			struct audio_effect_node_t * node = &filter->nodes[id];
			node->bypass = bypass ? 1 : 0;
		}
	}
}

void audio_filter_setup(struct audio_filter_t * filter, int id, const char * json, int len)
{
	if(filter)
	{
		if((id >= 0) && (id < filter->nlen))
		{
			struct audio_effect_node_t * node = &filter->nodes[id];
			if(json && (len > 0))
			{
				struct json_value_t * v = json_parse(json, len, NULL);
				if(v && (v->type == JSON_OBJECT))
					node->ae->setup(node->ctx, v);
				json_free(v);
			}
		}
	}
}

void audio_filter_bypass_by_name(struct audio_filter_t * filter, const char * name, int bypass)
{
	if(filter)
	{
		for(int i = 0; i < filter->nlen; i++)
		{
			struct audio_effect_node_t * node = &filter->nodes[i];
			if(xos_strcmp(node->ae->name, name) == 0)
				node->bypass = bypass ? 1 : 0;
		}
	}
}

void audio_filter_setup_by_name(struct audio_filter_t * filter, const char * name, const char * json, int len)
{
	if(filter)
	{
		for(int i = 0; i < filter->nlen; i++)
		{
			struct audio_effect_node_t * node = &filter->nodes[i];
			if(xos_strcmp(node->ae->name, name) == 0)
			{
				if(json && (len > 0))
				{
					struct json_value_t * v = json_parse(json, len, NULL);
					if(v && (v->type == JSON_OBJECT))
						node->ae->setup(node->ctx, v);
					json_free(v);
				}
			}
		}
	}
}

struct audio_frame_t * audio_filter_process(struct audio_filter_t * filter, struct audio_frame_t * input)
{
	struct audio_frame_t * output = input;

	if(filter && input)
	{
		for(int i = 0; i < filter->nlen; i++)
		{
			struct audio_effect_node_t * node = &filter->nodes[i];
			if(!node->bypass)
			{
				node->ae->prepare(node->ctx, output);
				output = node->ae->process(node->ctx, output);
			}
		}
	}
	return output;
}

static void audio_effect_pure_init(void)
{
	xos_mutex_init(&__audio_effect_lock);
}

static void audio_effect_pure_exit(void)
{
	xos_mutex_exit(&__audio_effect_lock);
}

pure_initcall(audio_effect_pure_init);
pure_exitcall(audio_effect_pure_exit);
