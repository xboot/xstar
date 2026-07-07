/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <xos/xos.h>
#include <driver/clocksource/clocksource.h>

struct pcg32_random_t {
	uint64_t state;
	uint64_t inc;
};
static struct pcg32_random_t pcg32_global = { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL };

static uint32_t pcg32_random_r(struct pcg32_random_t * rng)
{
	uint64_t oldstate = rng->state;
	rng->state = oldstate * 6364136223846793005ULL + rng->inc;
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static void pcg32_srandom_r(struct pcg32_random_t * rng, uint64_t state, uint64_t seq)
{
	rng->state = 0U;
	rng->inc = (seq << 1u) | 1u;
	pcg32_random_r(rng);
	rng->state += state;
	pcg32_random_r(rng);
}

static inline uint32_t pcg32_random(void)
{
	return pcg32_random_r(&pcg32_global);
}

static inline void pcg32_srandom(uint64_t state, uint64_t seq)
{
	pcg32_srandom_r(&pcg32_global, state, seq);
}

void xos_srand(unsigned int seed)
{
	uint64_t state = (uint64_t)ktime_to_ns(ktime_get()) ^ (uint64_t)seed;
	uint64_t seq = (uint64_t)seed;
	pcg32_srandom(state, seq);
}

int xos_rand(void)
{
	return (int)(pcg32_random() & XOS_RAND_MAX);
}

int xos_random_int(int lower, int upper)
{
	return (pcg32_random() % (upper - lower + 1)) + lower;
}

float xos_random_float(float lower, float upper)
{
	return ((float)pcg32_random() / (float)0xffffffff) * (upper - lower) + lower;
}

double xos_random_double(double lower, double upper)
{
	return ((double)pcg32_random() / (double)0xffffffff) * (upper - lower) + lower;
}
