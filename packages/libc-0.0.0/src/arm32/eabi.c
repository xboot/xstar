#include <xos/xos.h>

int raise(int signal)
{
	return 0;
}

void __aeabi_unwind_cpp_pr0(void)
{
}

void __aeabi_unwind_cpp_pr1(void)
{
}

void __aeabi_memcpy(void * dest, const void * src, size_t len)
{
	xos_memcpy(dest, src, len);
}

void __aeabi_memset(void * s, size_t n, int c)
{
	xos_memset(s, c, n);
}
