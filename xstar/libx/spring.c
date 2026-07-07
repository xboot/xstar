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

#include <libx/spring.h>

#define SPRING_MAX_DELTA_TIME	(0.083f)
#define SPRING_STEP_TIME		(0.001f)
#define SPRING_PRECISION		(0.005f)

void spring_init(struct spring_t * s, float start, float stop, float velocity, float tension, float friction)
{
	s->start = start;
	s->stop = stop;
	s->velocity = velocity;
	s->tension = tension;
	s->friction = friction;
	s->acc = 0;
}

int spring_step(struct spring_t * s, float dt)
{
	float v, p;

	if((fabs(s->velocity) <= SPRING_PRECISION) && (fabs(s->stop - s->start) <= SPRING_PRECISION))
		return 0;
	if(dt > SPRING_MAX_DELTA_TIME)
		dt = SPRING_MAX_DELTA_TIME;
	s->acc += dt;
	while(s->acc >= SPRING_STEP_TIME)
	{
		s->acc -= SPRING_STEP_TIME;
		v = s->velocity + (s->tension * (s->stop - s->start) - s->friction * s->velocity) * SPRING_STEP_TIME;
		p = s->start + v * SPRING_STEP_TIME;
		if((fabs(v) <= SPRING_PRECISION) && (fabs(s->stop - p) <= SPRING_PRECISION))
		{
			s->start = s->stop;
			s->velocity = 0;
			break;
		}
		else
		{
			s->start = p;
			s->velocity = v;
		}
	}
	return 1;
}
