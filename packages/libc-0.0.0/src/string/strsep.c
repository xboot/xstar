#include <xos/xos.h>
#include <string.h>

char * strsep(char ** s, const char * ct)
{
	return xos_strsep(s, ct);
}
