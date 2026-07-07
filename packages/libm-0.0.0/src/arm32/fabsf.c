float fabsf(float x)
{
	__asm__ __volatile__("vabs.f32 %0, %1" : "=t"(x) : "t"(x));
	return x;
}
