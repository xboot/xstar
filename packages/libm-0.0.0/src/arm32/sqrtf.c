float sqrtf(float x)
{
	__asm__ __volatile__("vsqrt.f32 %0, %1" : "=t"(x) : "t"(x));
	return x;
}
