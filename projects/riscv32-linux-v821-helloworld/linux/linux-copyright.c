#include <linux/linux.h>

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

char * linux_copyright_uniqueid(void)
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

int linux_copyright_verify(void)
{
	return 1;
}
