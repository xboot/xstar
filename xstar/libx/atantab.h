#ifndef __XSTAR_LIBX_ATANTAB_H__
#define __XSTAR_LIBX_ATANTAB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>
#include <math.h>
#include <libx/sqrti.h>

/*
 * Integer inverse trigonometric functions.
 *
 * Angles use 16-bit fixed-point units where a full turn (2*PI) equals 65536,
 * the same convention as isin()/icos() in costab.h, so a result from iatan2()
 * can be fed straight back into them. iatan2(y, x) and iatan(t) return a
 * signed angle; iasin(v)/iacos(v) take a unit-amplitude value scaled so that
 * 32767 == 1.0 (matching icostab).
 */
extern const int16_t iatantab[1025];

static inline int iatan(int t)
{
	int neg = 0;
	if(t < 0)
	{
		neg = 1;
		t = -t;
	}
	if(t > 1024)
		t = 1024;
	int n = t;
	int frac = t - n;
	int a = iatantab[n] + (((iatantab[n + 1] - iatantab[n]) * frac) >> 10);
	return neg ? -a : a;
}

static inline int iatan2(int y, int x)
{
	int num, den, phi;

	if((x == 0) && (y == 0))
		return 0;

	int ax = (x < 0) ? -x : x;
	int ay = (y < 0) ? -y : y;

	if(ay <= ax)
	{
		num = ay;
		den = ax;
	}
	else
	{
		num = ax;
		den = ay;
	}

	int n = (num << 10) / den;
	if(n > 1024)
		n = 1024;
	int frac = (num << 10) - n * den;
	int alpha = iatantab[n] + ((iatantab[n + 1] - iatantab[n]) * frac) / den;

	if(ay <= ax)
		phi = alpha;
	else
		phi = 16384 - alpha;

	if(x < 0)
		phi = 32768 - phi;
	if(y < 0)
		phi = -phi;
	return phi;
}

static inline int iasin(int v)
{
	if(v > 32767)
		v = 32767;
	if(v < -32767)
		v = -32767;
	int r = (v < 0) ? -v : v;
	r = (int)sqrti((unsigned long)32767 * 32767 - (unsigned long)r * r);
	return iatan2(v, r);
}

static inline int iacos(int v)
{
	if(v > 32767)
		v = 32767;
	if(v < -32767)
		v = -32767;
	int r = (v < 0) ? -v : v;
	r = (int)sqrti((unsigned long)32767 * 32767 - (unsigned long)r * r);
	return iatan2(r, v);
}

/*
 * Floating-point inverse trigonometric functions.
 *
 * Angles are returned in radians. fatan2(y, x) spans [-PI, PI]; fatan(t) is
 * clamped to t in [-1, 1] and returns [-PI/4, PI/4]; fasin(x)/facos(x) clamp
 * |x| <= 1 and return [-PI/2, PI/2] / [0, PI] respectively.
 */
extern const float fatantab[1025];

static inline float fatan(float t)
{
	int neg = 0;
	if(t < 0.0f)
	{
		neg = 1;
		t = -t;
	}
	if(t > 1.0f)
		t = 1.0f;
	float f = t * 1024.0f;
	int n = (int)f;
	f -= (float)n;
	float a = fatantab[n] + f * (fatantab[n + 1] - fatantab[n]);
	return neg ? -a : a;
}

static inline float fatan2(float y, float x)
{
	float num, den, phi;

	if((x == 0.0f) && (y == 0.0f))
		return 0.0f;

	float ax = (x < 0.0f) ? -x : x;
	float ay = (y < 0.0f) ? -y : y;

	if(ay <= ax)
	{
		num = ay;
		den = ax;
	}
	else
	{
		num = ax;
		den = ay;
	}

	float f = (num / den) * 1024.0f;
	int n = (int)f;
	if(n > 1024)
		n = 1024;
	f -= (float)n;
	float alpha = fatantab[n] + f * (fatantab[n + 1] - fatantab[n]);

	if(ay <= ax)
		phi = alpha;
	else
		phi = (float)(1.5707963267948966f) - alpha;

	if(x < 0.0f)
		phi = (float)(3.1415926535897932f) - phi;
	if(y < 0.0f)
		phi = -phi;
	return phi;
}

static inline float fasin(float x)
{
	float c = 1.0f - x * x;
	if(c < 0.0f)
		c = 0.0f;
	return fatan2(x, sqrtf(c));
}

static inline float facos(float x)
{
	float c = 1.0f - x * x;
	if(c < 0.0f)
		c = 0.0f;
	return fatan2(sqrtf(c), x);
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_ATANTAB_H__ */
