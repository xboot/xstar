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
#include <libx/sarg.h>

static inline int sarg_isopt(const char * tok)
{
	return (tok[0] == '-') && (tok[1] != '\0') && ((tok[1] == '-') || xos_isalpha((unsigned char)tok[1]));
}

static inline int sarg_optend(struct sarg_t * sarg)
{
	for(int i = 1; i < sarg->argc; i++)
	{
		if(xos_strcmp(sarg->argv[i], "--") == 0)
			return i;
	}
	return sarg->argc;
}

void sarg_init(struct sarg_t * sarg, int argc, char ** argv)
{
	if(sarg)
	{
		sarg->argc = argc;
		sarg->argv = argv;
		sarg->iter.index = 1;
		sarg->iter.literal = 0;
	}
}

int sarg_valid(struct sarg_t * sarg, const char ** opts, int min, int max)
{
	if(sarg)
	{
		if(min < 0)
			min = 0;
		int pos = 0, literal = 0;
		for(int i = 1; i < sarg->argc; i++)
		{
			char * tok = sarg->argv[i];
			if(!literal && (xos_strcmp(tok, "--") == 0))
			{
				literal = 1;
				continue;
			}
			if(!literal && sarg_isopt(tok))
			{
				int ok = 0;
				if(opts)
				{
					for(const char ** p = opts; *p; p++)
					{
						const char * name = *p;
						int n = (int)xos_strlen(name);
						if((n > 0) && (xos_strncmp(tok, name, n) == 0) && ((tok[n] == '\0') || (tok[n] == '=')))
						{
							ok = 1;
							break;
						}
					}
				}
				if(!ok)
					return 0;
				continue;
			}
			pos++;
		}
		if((pos < min) || ((max >= 0) && (pos > max)))
			return 0;
		return 1;
	}
	return 0;
}

int sarg_has(struct sarg_t * sarg, const char * name)
{
	if(sarg && name)
	{
		int n = (int)xos_strlen(name);
		int end = sarg_optend(sarg);
		for(int i = 1; i < end; i++)
		{
			char * tok = sarg->argv[i];
			if((n > 0) && (xos_strncmp(tok, name, n) == 0) && ((tok[n] == '\0') || (tok[n] == '=')))
				return 1;
		}
	}
	return 0;
}

const char * sarg_at(struct sarg_t * sarg, int index)
{
	if(sarg)
	{
		int pos = 0, literal = 0;
		for(int i = 1; i < sarg->argc; i++)
		{
			char * tok = sarg->argv[i];
			if(!literal && (xos_strcmp(tok, "--") == 0))
			{
				literal = 1;
				continue;
			}
			if(!literal && sarg_isopt(tok))
				continue;
			if(pos == index)
				return tok;
			pos++;
		}
	}
	return NULL;
}

const char * sarg_get(struct sarg_t * sarg, const char * name, const char * def)
{
	if(sarg && name)
	{
		int n = (int)xos_strlen(name);
		int end = sarg_optend(sarg);
		for(int i = end - 1; i >= 1; i--)
		{
			char * tok = sarg->argv[i];
			if((n > 0) && (xos_strncmp(tok, name, n) == 0) && (tok[n] == '='))
				return tok + n + 1;
		}
	}
	return def;
}

int sarg_get_int(struct sarg_t * sarg, const char * name, int def)
{
	const char * v = sarg_get(sarg, name, NULL);
	if(v && *v)
	{
		char * end;
		long val = xos_strtol(v, &end, 0);
		if((end != v) && (*end == '\0'))
			return (int)val;
	}
	return def;
}

unsigned int sarg_get_uint(struct sarg_t * sarg, const char * name, unsigned int def)
{
	const char * v = sarg_get(sarg, name, NULL);
	if(v && *v && (*v != '-'))
	{
		char * end;
		unsigned long val = xos_strtoul(v, &end, 0);
		if((end != v) && (*end == '\0'))
			return (unsigned int)val;
	}
	return def;
}

long sarg_get_long(struct sarg_t * sarg, const char * name, long def)
{
	const char * v = sarg_get(sarg, name, NULL);
	if(v && *v)
	{
		char * end;
		long val = xos_strtol(v, &end, 0);
		if((end != v) && (*end == '\0'))
			return val;
	}
	return def;
}

unsigned long sarg_get_ulong(struct sarg_t * sarg, const char * name, unsigned long def)
{
	const char * v = sarg_get(sarg, name, NULL);
	if(v && *v && (*v != '-'))
	{
		char * end;
		unsigned long val = xos_strtoul(v, &end, 0);
		if((end != v) && (*end == '\0'))
			return val;
	}
	return def;
}

double sarg_get_double(struct sarg_t * sarg, const char * name, double def)
{
	const char * v = sarg_get(sarg, name, NULL);
	if(v && *v)
	{
		char * end;
		double val = xos_strtod(v, &end);
		if((end != v) && (*end == '\0'))
			return val;
	}
	return def;
}

void sarg_iter_reset(struct sarg_t * sarg)
{
	if(sarg)
	{
		sarg->iter.index = 1;
		sarg->iter.literal = 0;
	}
}

const char * sarg_iter_next(struct sarg_t * sarg)
{
	if(sarg)
	{
		while(sarg->iter.index < sarg->argc)
		{
			char * tok = sarg->argv[sarg->iter.index];
			if(!sarg->iter.literal && (xos_strcmp(tok, "--") == 0))
			{
				sarg->iter.literal = 1;
				sarg->iter.index++;
				continue;
			}
			if(!sarg->iter.literal && sarg_isopt(tok))
			{
				sarg->iter.index++;
				continue;
			}
			sarg->iter.index++;
			return tok;
		}
	}
	return NULL;
}
