#include <xos/xos.h>
#include <assert.h>

void __assert_fail(const char * expr, const char * file, int line, const char * func)
{
	xos_printf("Assertion failed: %s (%s: %s: %d)\r\n", expr, file, func, line);
}
