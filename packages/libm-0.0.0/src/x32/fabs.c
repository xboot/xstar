double fabs(double x)
{
	__asm__ __volatile__("fabs" : "+t"(x));
	return x;
}
