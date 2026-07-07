#include <xos/xos.h>
#include <string.h>

size_t strcspn(const char * s, const char * reject)
{
	return xos_strcspn(s, reject);
}
