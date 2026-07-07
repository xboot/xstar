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
#include <libx/winfunc.h>

void winfunc_rectangular(float * taps, int n)
{
	for(int i = 0; i < n; i++)
		taps[i] = 1.0;
}

void winfunc_triangular(float * taps, int n)
{
	int rem = n + (n % 2);
	int initial = -(n - 1);

	for(int i = 0; i < n; i++)
	{
		taps[i] = 1 - fabsf((float)initial / (float)rem);
		initial += 2;
	}
}

void winfunc_flattop(float * taps, int n)
{
	float a0 = 0.21557895;
	float a1 = 0.41663158;
	float a2 = 0.277263158;
	float a3 = 0.083578947;
	float a4 = 0.006947368;
	float factor = 2 * M_PI / (n - 1);

	for(int i = 0; i < n; i++)
	{
		float t = factor * i;
		taps[i] = a0 - a1 * cosf(t) + a2 * cosf(2 * t) - a3 * cosf(3 * t) + a4 * cosf(4 * t);
	}
}

void winfunc_welch(float * taps, int n)
{
	for(int i = 0; i < n; i++)
	{
		float t = ((float)i - 0.5 * (float)(n - 1)) / (0.5 * (float)(n + 1));
		taps[i] = (1.0 - t * t);
	}
}

void winfunc_hanning(float * taps, int n)
{
	for(int i = 0; i < n; i++)
		taps[i] = 0.5 - 0.5 * cosf((2 * M_PI * i) / (n - 1));
}

void winfunc_hamming(float * taps, int n)
{
	for(int i = 0; i < n; i++)
		taps[i] = 0.54 - 0.46 * cosf((2 * M_PI * i) / (n - 1));
}

void winfunc_blackman(float * taps, int n)
{
	for(int i = 0; i < n; i++)
		taps[i] = 0.42 - 0.5 * cosf((2 * M_PI * i) / (n - 1)) + 0.08 * cosf((4 * M_PI * i) / (n - 1));
}

static float kaiser_izero(float x)
{
	float sum, u, halfx, temp;
	int n;

	sum = u = n = 1;
	halfx = x / 2.0;
	do {
		temp = halfx / (float)n;
		n += 1;
		temp *= temp;
		u *= temp;
		sum += u;
	} while(u >= (1e-21 * sum));
	return (sum);
}

void winfunc_kaiser(float * taps, int n, float beta)
{
	float i_beta = 1.0 / kaiser_izero(beta);
	float inm1 = 1.0 / ((float)(n - 1));

	taps[0] = i_beta;
	for(int i = 1; i < n - 1; i++)
	{
		float temp = 2 * i * inm1 - 1;
		taps[i] = kaiser_izero(beta * sqrtf(1.0 - temp * temp)) * i_beta;
	}
	taps[n - 1] = i_beta;
}
