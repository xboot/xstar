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

#include <xos/xos.h>
#include <libx/cron.h>

static const char * month_names[] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

static const char * week_names[] = {
	"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

static int cron_parse_value(const char ** pp, const char ** names, int name_count, int name_offset, int * out)
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

static int cron_parse_field(const char * field, int min, int max, const char ** names, int name_count, int name_offset, uint64_t * out)
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
			if(!cron_parse_value(&p, names, name_count, name_offset, &low))
				return 0;
			high = low;
		}
		if(*p == '-')
		{
			p++;
			if(!cron_parse_value(&p, names, name_count, name_offset, &high))
				return 0;
			has_high = 1;
		}
		if(*p == '/')
		{
			p++;
			if(!cron_parse_value(&p, NULL, 0, 0, &step))
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

int cron_parse(struct cron_t * cron, const char * expr)
{
	if(cron && expr)
	{
		char * dup = xos_strdup(expr);
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
			if(!cron_parse_field(f[0], 0, 59, NULL, 0, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			cron->minute = tmp;
			if(!cron_parse_field(f[1], 0, 23, NULL, 0, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			cron->hour = (uint32_t)tmp;
			if(!cron_parse_field(f[2], 1, 31, NULL, 0, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			cron->day = (uint32_t)tmp;
			if(!cron_parse_field(f[3], 1, 12, month_names, 12, 1, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			cron->month = (uint16_t)tmp;
			if(!cron_parse_field(f[4], 0, 7, week_names, 7, 0, &tmp))
			{
				xos_mem_free(dup);
				return 0;
			}
			if(tmp & ((uint64_t)0x1 << 7))
				tmp |= 0x1;
			tmp &= ~((uint64_t)0x1 << 7);
			cron->week = (uint8_t)tmp;
			cron->restricted = 0;
			if(xos_strcmp(f[2], "*") != 0)
				cron->restricted |= (0x1 << 0);
			if(xos_strcmp(f[4], "*") != 0)
				cron->restricted |= (0x1 << 1);
			xos_mem_free(dup);
			return 1;
		}
	}
	return 0;
}

int cron_match(struct cron_t * cron, int minute, int hour, int day, int month, int week)
{
	if(!cron)
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
	if(!((cron->minute >> minute) & 0x1))
		return 0;
	if(!((cron->hour >> hour) & 0x1))
		return 0;
	if(!((cron->month >> month) & 0x1))
		return 0;
	if((cron->restricted & 0x3) == 0x3)
		return (((cron->day >> day) & 0x1) || ((cron->week >> week) & 0x1)) ? 1 : 0;
	else
		return (((cron->day >> day) & 0x1) && ((cron->week >> week) & 0x1)) ? 1 : 0;
}
