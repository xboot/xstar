#include <xos/xos.h>
#include <ctype.h>

int iscntrl(int c)
{
	return xos_iscntrl(c);
}
