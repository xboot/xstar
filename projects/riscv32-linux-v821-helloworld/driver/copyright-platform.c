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

static char * strim(char * s)
{
	if(s)
	{
		while(isspace(*s))
			s++;
		if(*s == 0)
			return s;
		char * e = s + strlen(s) - 1;
		while((e > s) && isspace(*e))
			e--;
		*(e + 1) = 0;
	}
	return s;
}

static const char * platform_uniqueid(void)
{
	static char uniqueid[32 + 1] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	FILE * fp = fopen("/sys/class/sunxi_info/sys_info", "r");
	if(fp)
	{
		char entry[1024];
		while(fgets(entry, sizeof(entry), fp) != NULL)
		{
			char * r = entry;
			if(strchr(r, ':'))
			{
				char * k = strim(strsep(&r, ":"));
				char * v = strim(r);
				k = (k && (*k != '\0')) ? k : NULL;
				v = (v && (*v != '\0')) ? v : NULL;
				if(k && v)
				{
					if((strcmp(k, "sunxi_serial") == 0) && (strlen(v) >= 32))
					{
						memcpy(&uniqueid[0], &v[24], 8);
						memcpy(&uniqueid[8], &v[16], 8);
						memcpy(&uniqueid[16], &v[8], 8);
						memcpy(&uniqueid[24], &v[0], 8);
						uniqueid[32] = '\0';
					}
				}
			}
		}
		fclose(fp);
	}
	return uniqueid;
}

static int platform_verify(void)
{
	return 1;
}

static int platform_keygen(const char * msg, void * key, int maxlen)
{
	return 0;
}

static struct copyright_t copyright_platform = {
	.uniqueid	= platform_uniqueid,
	.keygen		= platform_keygen,
	.verify		= platform_verify,
};

static void copyright_platform_init(void)
{
	register_copyright(&copyright_platform);
}
core_initcall(copyright_platform_init);
