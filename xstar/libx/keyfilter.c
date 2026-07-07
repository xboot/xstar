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

#include <libx/xdef.h>
#include <libx/keyfilter.h>

void keyfilter_init(struct keyfilter_t * filter)
{
	if(filter)
	{
		filter->head = 0;
		filter->tail = 0;
		filter->pressed = -1;
		filter->timeout.click = 200;
		filter->timeout.longpress = 500;
		filter->timeout.repeat = 100;
		filter->timestamp = ms_to_ktime(0);
		filter->repeatlast = ms_to_ktime(0);
		filter->state = KEYFILTER_STATE_IDLE;
		filter->pending = KEYFILTER_TYPE_NONE;
	}
}

void keyfilter_settimeout(struct keyfilter_t * filter, int click, int longpress, int repeat)
{
	if(filter)
	{
		filter->timeout.click = XMAX(20, click);
		filter->timeout.longpress = XMAX(50, longpress);
		filter->timeout.repeat = XMAX(0, repeat);
	}
}

int keyfilter_feed(struct keyfilter_t * filter, ktime_t timestamp, int pressed)
{
	if(filter)
	{
		if(filter->pressed == pressed)
			return 1;
		int next = (filter->head + 1) & (ARRAY_SIZE(filter->queue) - 1);
		if(next == filter->tail)
			return 0;
		filter->queue[filter->head].timestamp = timestamp;
		filter->queue[filter->head].pressed = pressed;
		filter->head = next;
		filter->pressed = pressed;
		return 1;
	}
	return 0;
}

enum keyfilter_type_t keyfilter_poll(struct keyfilter_t * filter, ktime_t now)
{
	enum keyfilter_type_t type = KEYFILTER_TYPE_NONE;

	if(filter)
	{
		if(filter->pending == KEYFILTER_TYPE_NONE)
		{
			struct keyfilter_entry_t * e = NULL;
			if(filter->tail != filter->head)
			{
				e = &filter->queue[filter->tail];
				filter->tail = (filter->tail + 1) & (ARRAY_SIZE(filter->queue) - 1);
			}
			switch(filter->state)
			{
			case KEYFILTER_STATE_IDLE:
			{
				if(e && e->pressed)
				{
					filter->timestamp = e->timestamp;
					filter->state = KEYFILTER_STATE_PRESSED;
					type = KEYFILTER_TYPE_DOWN;
				}
				break;
			}
			case KEYFILTER_STATE_PRESSED:
			{
				if(e && !e->pressed)
				{
					filter->timestamp = e->timestamp;
					filter->state = KEYFILTER_STATE_RELEASED;
					type = KEYFILTER_TYPE_UP;
				}
				else
				{
					if(ktime_after(now, ktime_add_ms(filter->timestamp, filter->timeout.longpress)))
					{
						filter->state = KEYFILTER_STATE_HELD;
						filter->repeatlast = now;
						type = KEYFILTER_TYPE_LONGPRESS;
					}
				}
				break;
			}
			case KEYFILTER_STATE_RELEASED:
			{
				if(e && e->pressed)
				{
					filter->timestamp = e->timestamp;
					filter->state = KEYFILTER_STATE_REPRESSED;
					type = KEYFILTER_TYPE_DOWN;
				}
				else
				{
					if(ktime_after(now, ktime_add_ms(filter->timestamp, filter->timeout.click)))
					{
						filter->state = KEYFILTER_STATE_IDLE;
						type = KEYFILTER_TYPE_CLICK;
					}
				}
				break;
			}
			case KEYFILTER_STATE_REPRESSED:
			{
				if(e && !e->pressed)
				{
					filter->state = KEYFILTER_STATE_IDLE;
					filter->pending = KEYFILTER_TYPE_DBLCLICK;
					type = KEYFILTER_TYPE_UP;
				}
				else
				{
					if(ktime_after(now, ktime_add_ms(filter->timestamp, filter->timeout.longpress)))
					{
						filter->state = KEYFILTER_STATE_HELD;
						filter->repeatlast = now;
						type = KEYFILTER_TYPE_LONGPRESS;
					}
				}
				break;
			}
			case KEYFILTER_STATE_HELD:
			{
				if(e && !e->pressed)
				{
					filter->state = KEYFILTER_STATE_IDLE;
					type = KEYFILTER_TYPE_UP;
				}
				else
				{
					if((filter->timeout.repeat > 0) && ktime_after(now, ktime_add_ms(filter->repeatlast, filter->timeout.repeat)))
					{
						filter->repeatlast = now;
						type = KEYFILTER_TYPE_REPEAT;
					}
				}
				break;
			}
			default:
				break;
			}
		}
		else
		{
			type = filter->pending;
			filter->pending = KEYFILTER_TYPE_NONE;
		}
	}
	return type;
}

void keyfilter_clear(struct keyfilter_t * filter)
{
	if(filter)
	{
		filter->head = 0;
		filter->tail = 0;
		filter->pressed = -1;
		filter->timestamp = ms_to_ktime(0);
		filter->repeatlast = ms_to_ktime(0);
		filter->state = KEYFILTER_STATE_IDLE;
		filter->pending = KEYFILTER_TYPE_NONE;
	}
}
