#include <xos/xos.h>
#include <ctype.h>

int isalnum(int c)
{
	return xos_isalpha(c);
}
