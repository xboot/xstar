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

#include <xstar.h>
#include <kernel/core/cron.h>

/*
 * Standard 5-field cron expression
 *   *    *    *    *    *
 *   │    │    │    │    │
 *   │    │    │    │    └── day of week (0 - 6, 0 = Sunday)
 *   │    │    │    └─────── month (1 - 12)
 *   │    │    └──────────── day of month (1 - 31)
 *   │    └───────────────── hour (0 - 23)
 *   └────────────────────── minute (0 - 59)
 */
struct cron_rule_t {
	uint64_t minute;
	uint32_t hour;
	uint32_t day;
	uint16_t month;
	uint8_t week;
	uint8_t restricted;
};

static const char * month_names[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char * week_names[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static int cron_rule_parse_value(const char ** pp, const char ** names, int name_count, int name_offset, int * out)
{
	const char * p = *pp;

	if(xos_isdigit((unsigned char)*p))
	{
		char * end;
		long v = xos_strtol(p, &end, 10);
		if(end == p)
			return 0;
		*pp = end;
		*out = (int)v;
		return 1;
	}
	if(names && xos_isalpha((unsigned char)*p))
	{
		for(int i = 0; i < name_count; i++)
		{
			if(xos_strncasecmp(p, names[i], 3) == 0)
			{
				int n = 0;
				while(xos_isalpha((unsigned char)p[n]))
					n++;
				*pp = p + n;
				*out = i + name_offset;
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

static int cron_rule_parse_field(const char * field, int min, int max, const char ** names, int name_count, int name_offset, uint64_t * out)
{
	const char * p = field;
	uint64_t bits = 0;

	if(!field || !*field)
		return 0;
	while(*p)
	{
		int low, high, step = 1, has_high = 0;
		while(xos_isspace((unsigned char)*p))
			p++;
		if(*p == '\0')
			break;
		if(*p == '*')
		{
			low = min;
			high = max;
			has_high = 1;
			p++;
		}
		else
		{
			if(!cron_rule_parse_value(&p, names, name_count, name_offset, &low))
				return 0;
			high = low;
		}
		if(*p == '-')
		{
			p++;
			if(!cron_rule_parse_value(&p, names, name_count, name_offset, &high))
				return 0;
			has_high = 1;
		}
		if(*p == '/')
		{
			p++;
			if(!cron_rule_parse_value(&p, NULL, 0, 0, &step))
				return 0;
			if(step <= 0)
				return 0;
			if(!has_high)
				high = max;
		}
		if(low < min || high > max || low > high)
			return 0;
		for(int i = low; i <= high; i += step)
			bits |= ((uint64_t)0x1 << i);
		while(xos_isspace((unsigned char)*p))
			p++;
		if(*p == ',')
		{
			p++;
			continue;
		}
		if(*p == '\0')
			break;
		return 0;
	}
	if(bits == 0)
		return 0;
	*out = bits;
	return 1;
}

static int cron_rule_parse(struct cron_rule_t * rule, const char * spec)
{
	if(rule && spec)
	{
		char * dup = xos_strdup(spec);
		if(dup)
		{
			char * f[5];
			char * save = NULL;
			uint64_t tmp;
			for(int i = 0; i < 5; i++)
			{
				f[i] = xos_strtok(i ? NULL : dup, " \t", &save);
				if(!f[i])
				{
					xos_mem_free(dup);
					return 0;
				}
			}
			if(xos_strtok(NULL, " \t", &save) != NULL)
			{
				xos_mem_free(dup);
				return 0;
			}
			if(!cron_rule_parse_field(f[0], 0, 59, NULL, 0, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			rule->minute = tmp;
			if(!cron_rule_parse_field(f[1], 0, 23, NULL, 0, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			rule->hour = (uint32_t)tmp;
			if(!cron_rule_parse_field(f[2], 1, 31, NULL, 0, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			rule->day = (uint32_t)tmp;
			if(!cron_rule_parse_field(f[3], 1, 12, month_names, 12, 1, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			rule->month = (uint16_t)tmp;
			if(!cron_rule_parse_field(f[4], 0, 7, week_names, 7, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			if(tmp & ((uint64_t)0x1 << 7))
				tmp |= 0x1;
			tmp &= ~((uint64_t)0x1 << 7);
			rule->week = (uint8_t)tmp;
			rule->restricted = 0;
			if(xos_strcmp(f[2], "*") != 0)
				rule->restricted |= (0x1 << 0);
			if(xos_strcmp(f[4], "*") != 0)
				rule->restricted |= (0x1 << 1);
			xos_mem_free(dup);
			return 1;
		}
	}
	return 0;
}

static int cron_rule_match(struct cron_rule_t * rule, int minute, int hour, int day, int month, int week)
{
	if(!rule)
		return 0;
	if((minute < 0) || (minute > 59))
		return 0;
	if((hour < 0) || (hour > 23))
		return 0;
	if((day < 1) || (day > 31))
		return 0;
	if((month < 1) || (month > 12))
		return 0;
	if((week < 0) || (week > 6))
		return 0;
	if(!((rule->minute >> minute) & 0x1))
		return 0;
	if(!((rule->hour >> hour) & 0x1))
		return 0;
	if(!((rule->month >> month) & 0x1))
		return 0;
	if((rule->restricted & 0x3) == 0x3)
		return (((rule->day >> day) & 0x1) || ((rule->week >> week) & 0x1)) ? 1 : 0;
	else
		return (((rule->day >> day) & 0x1) && ((rule->week >> week) & 0x1)) ? 1 : 0;
}

struct cron_job_t {
	struct list_head_t entry;
	struct cron_rule_t rule;
	char * name;
	char * spec;
	void (*exec)(void *);
	void (*destroy)(void *);
	void * data;
	int oneshot;
	int refcount;
	int removed;
	uint32_t last;
};

static void cron_tick(struct cron_t * cron, struct wallclock_time_t * tm)
{
	uint32_t now = ((tm->year & 0x7ff) << 21) | (tm->month << 16) | (tm->day << 11) | (tm->hour << 6) | (tm->minute << 0);
	xos_mutex_lock(&cron->lock);
	{
		struct cron_job_t * pos;
		int n = 0;
		list_for_each_entry(pos, &cron->head, entry)
			n++;
		if(n > 0)
		{
			struct cron_job_t ** hits = xos_mem_malloc(sizeof(struct cron_job_t *) * n);
			if(hits)
			{
				int h = 0;
				list_for_each_entry(pos, &cron->head, entry)
				{
					if(pos->removed)
						continue;
					if((pos->last != now) && cron_rule_match(&pos->rule, tm->minute, tm->hour, tm->day, tm->month, tm->week))
					{
						pos->last = now;
						pos->refcount++;
						hits[h++] = pos;
					}
				}
				xos_mutex_unlock(&cron->lock);

				for(int i = 0; i < h; i++)
				{
					if(hits[i]->exec)
						hits[i]->exec(hits[i]->data);
					xos_mutex_lock(&cron->lock);
					int drop = --hits[i]->refcount;
					if(hits[i]->oneshot && !hits[i]->removed)
					{
						list_del(&hits[i]->entry);
						hits[i]->removed = 1;
						drop = --hits[i]->refcount;
					}
					int removed = hits[i]->removed;
					xos_mutex_unlock(&cron->lock);
					if(removed && (drop == 0))
					{
						if(hits[i]->name)
							xos_mem_free(hits[i]->name);
						if(hits[i]->destroy)
							hits[i]->destroy(hits[i]->data);
						xos_mem_free(hits[i]);
					}
				}
				xos_mem_free(hits);
				return;
			}
		}
	}
	xos_mutex_unlock(&cron->lock);
}

static void cron_thread(void * data)
{
	struct cron_t * cron = (struct cron_t *)data;
	struct wallclock_time_t tm;

	while(cron->running)
	{
		wallclock_gettime(&tm, cron->tz);
		xos_semaphore_wait(&cron->sem, (60 - tm.second) * 1000);
		if(!cron->running)
			break;
		wallclock_gettime(&tm, cron->tz);
		cron_tick(cron, &tm);
	}
}

struct cron_t * cron_alloc(const char * tz)
{
	if(xstar_feature_thread())
	{
		struct cron_t * cron = xos_mem_malloc(sizeof(struct cron_t));
		if(cron)
		{
			init_list_head(&cron->head);
			xos_mutex_init(&cron->lock);
			xos_semaphore_init(&cron->sem, 0);
			cron->tz = tz ? xos_strdup(tz) : NULL;
			cron->running = 1;
			cron->thread = xos_thread_create(NULL, cron_thread, cron, 0);
			if(!cron->thread)
			{
				if(cron->tz)
					xos_mem_free(cron->tz);
				xos_semaphore_exit(&cron->sem);
				xos_mutex_exit(&cron->lock);
				xos_mem_free(cron);
				return NULL;
			}
			return cron;
		}
	}
	return NULL;
}

void cron_free(struct cron_t * cron)
{
	if(cron)
	{
		xos_mutex_lock(&cron->lock);
		cron->running = 0;
		xos_mutex_unlock(&cron->lock);
		xos_semaphore_post(&cron->sem);
		xos_thread_wait(cron->thread);
		xos_thread_destroy(cron->thread);
		xos_mutex_lock(&cron->lock);
		{
			struct cron_job_t * pos, * n;
			list_for_each_entry_safe(pos, n, &cron->head, entry)
			{
				list_del(&pos->entry);
				if(pos->name)
					xos_mem_free(pos->name);
				if(pos->destroy)
					pos->destroy(pos->data);
				xos_mem_free(pos);
			}
		}
		xos_mutex_unlock(&cron->lock);
		if(cron->tz)
			xos_mem_free(cron->tz);
		xos_semaphore_exit(&cron->sem);
		xos_mutex_exit(&cron->lock);
		xos_mem_free(cron);
	}
}

int cron_add(struct cron_t * cron, const char * name, const char * spec, int oneshot, void (*exec)(void *), void (*destroy)(void *), void * data)
{
	if(cron && name && spec && exec)
	{
		struct cron_job_t * job = xos_mem_malloc(sizeof(struct cron_job_t));
		if(job)
		{
			init_list_head(&job->entry);
			job->name = xos_strdup(name);
			job->spec = xos_strdup(spec);
			if(!job->name || !job->spec)
			{
				if(job->name)
					xos_mem_free(job->name);
				if(job->spec)
					xos_mem_free(job->spec);
				xos_mem_free(job);
				return 0;
			}
			if(!cron_rule_parse(&job->rule, spec))
			{
				xos_mem_free(job->name);
				xos_mem_free(job->spec);
				xos_mem_free(job);
				return 0;
			}
			job->exec = exec;
			job->destroy = destroy;
			job->data = data;
			job->oneshot = oneshot ? 1 : 0;
			job->refcount = 1;
			job->removed = 0;
			job->last = 0;

			xos_mutex_lock(&cron->lock);
			{
				struct cron_job_t * pos, * n;
				list_for_each_entry_safe(pos, n, &cron->head, entry)
				{
					if(!pos->removed && (xos_strcmp(pos->name, name) == 0))
					{
						xos_mutex_unlock(&cron->lock);
						xos_mem_free(job->name);
						xos_mem_free(job->spec);
						xos_mem_free(job);
						return 0;
					}
				}
				list_add_tail(&job->entry, &cron->head);
			}
			xos_mutex_unlock(&cron->lock);
			return 1;
		}
	}
	return 0;
}

int cron_remove(struct cron_t * cron, const char * name)
{
	if(cron && name)
	{
		xos_mutex_lock(&cron->lock);
		{
			struct cron_job_t * job = NULL;
			struct cron_job_t * pos, * n;
			list_for_each_entry_safe(pos, n, &cron->head, entry)
			{
				if(!pos->removed && (xos_strcmp(pos->name, name) == 0))
				{
					job = pos;
					break;
				}
			}
			if(job)
			{
				list_del(&job->entry);
				job->removed = 1;
				int drop = --job->refcount;
				xos_mutex_unlock(&cron->lock);
				if(drop == 0)
				{
					xos_mem_free(job->name);
					xos_mem_free(job->spec);
					if(job->destroy)
						job->destroy(job->data);
					xos_mem_free(job);
				}
				return 1;
			}
		}
		xos_mutex_unlock(&cron->lock);
	}
	return 0;
}

void cron_clear(struct cron_t * cron)
{
	if(cron)
	{
		xos_mutex_lock(&cron->lock);
		{
			struct cron_job_t * pos, * n;
			list_for_each_entry_safe(pos, n, &cron->head, entry)
			{
				list_del(&pos->entry);
				pos->removed = 1;
				int drop = --pos->refcount;
				if(drop == 0)
				{
					xos_mem_free(pos->name);
					xos_mem_free(pos->spec);
					if(pos->destroy)
						pos->destroy(pos->data);
					xos_mem_free(pos);
				}
			}
		}
		xos_mutex_unlock(&cron->lock);
	}
}

void cron_foreach(struct cron_t * cron, void (*cb)(char * name, char * spec, int oneshot, void * data), void * data)
{
	if(cron && cb)
	{
		xos_mutex_lock(&cron->lock);
		{
			struct cron_job_t * pos, * n;
			list_for_each_entry_safe(pos, n, &cron->head, entry)
			{
				if(!pos->removed)
					cb(pos->name, pos->spec, pos->oneshot, data);
			}
		}
		xos_mutex_unlock(&cron->lock);
	}
}
