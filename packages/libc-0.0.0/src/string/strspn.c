#include <xos/xos.h>
#include <string.h>

size_t strspn(const char * s, const char * accept)
{
	return xos_strspn(s, accept);
}
