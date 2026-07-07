#ifndef __XSTAR_DRIVER_CLOCKEVENT_H__
#define __XSTAR_DRIVER_CLOCKEVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct clockevent_t {
	char * name;
	uint32_t mult;
	uint32_t shift;
	uint64_t min_delta_ns;
	uint64_t max_delta_ns;
	void * data;
	void (*handler)(struct clockevent_t * ce, void * data);

	int (*next)(struct clockevent_t * ce, uint64_t evt);
	void * priv;
};

/*
 * clockevent_hz2mult - calculates mult from hz and shift
 * @hz: Clockevent frequency in Hz
 * @shift_constant:	Clockevent shift factor
 *
 * Helper functions that converts a hz counter
 * frequency to a timsource multiplier, given the
 * clockevent shift value
 */
static inline uint32_t clockevent_hz2mult(uint32_t hz, uint32_t shift)
{
	/*
	 * hz = cyc/(Billion ns)
	 * mult/2^shift  = ns/cyc
	 * mult = ns/cyc * 2^shift
	 * mult = 1Billion/hz * 2^shift
	 * mult = 1000000000 * 2^shift / hz
	 * mult = (1000000000<<shift) / hz
	 */
	uint64_t tmp = ((uint64_t)1000000000) << shift;
	tmp += hz/2;
	tmp = tmp / hz;
	return (uint32_t)tmp;
}

/*
 * clockevent_khz2mult - calculates mult from khz and shift
 * @khz: Clockevent frequency in KHz
 * @shift_constant: Clockevent shift factor
 *
 * Helper functions that converts a khz counter frequency to a timsource
 * multiplier, given the clockevent shift value
 */
static inline uint32_t clockevent_khz2mult(uint32_t khz, uint32_t shift)
{
	/*
	 * khz = cyc/(Million ns)
	 * mult/2^shift  = ns/cyc
	 * mult = ns/cyc * 2^shift
	 * mult = 1Million/khz * 2^shift
	 * mult = 1000000 * 2^shift / khz
	 * mult = (1000000<<shift) / khz
	 */
	uint64_t tmp = ((uint64_t)1000000) << shift;
	tmp += khz / 2;
	tmp = tmp / khz;
	return (uint32_t)tmp;
}

/*
 * clockevent_calc_mult_shift - calculate mult/shift factors from frequency and minsec
 * @ce:		pointer to clockevent
 * @freq:	frequency to convert to
 * @minsec:	guaranteed runtime conversion range in seconds
 *
 * The function evaluates the shift/mult pair for the scaled math
 * operations of clockevents.
 */
static inline void clockevent_calc_mult_shift(struct clockevent_t * ce, uint32_t freq, uint32_t minsec)
{
	uint64_t tmp;
	uint32_t sft, sftacc= 32;

	/*
	 * Calculate the shift factor which is limiting the conversion range
	 */
	tmp = ((uint64_t)minsec * 1000000000ULL) >> 32;
	while(tmp)
	{
		tmp >>=1;
		sftacc--;
	}

	/*
	 * Find the conversion shift/mult pair which has the best
	 * accuracy and fits the minsec conversion range:
	 */
	for(sft = 32; sft > 0; sft--)
	{
		tmp = (uint64_t) freq << sft;
		tmp += 1000000000ULL / 2;
		tmp = tmp / 1000000000ULL;
		if((tmp >> sftacc) == 0)
			break;
	}
	ce->mult = tmp;
	ce->shift = sft;
}

/*
 * clockevent_delta2ns - Convert a latch value (device ticks) to nanoseconds
 * @ce:	pointer to clockevent device
 * @latch:	value to convert
 *
 * Math helper, returns latch value converted to nanoseconds (bound checked)
 */
static inline uint64_t clockevent_delta2ns(struct clockevent_t * ce, uint64_t latch)
{
	uint64_t clc = (uint64_t)latch << ce->shift;
	uint64_t rnd;

	if(!ce->mult)
		ce->mult = 1;
	rnd = (uint64_t)ce->mult - 1;

	if((clc >> ce->shift) != (uint64_t)latch)
		clc = ~0ULL;

	if((~0ULL - clc > rnd) && (ce->mult <= (1ULL << ce->shift)))
		clc += rnd;

	clc = clc / ce->mult;
	return clc > 1000 ? clc : 1000;
}

struct clockevent_t * search_clockevent(const char * name);
struct clockevent_t * search_first_clockevent(void);
struct device_t * register_clockevent(struct clockevent_t * ce, struct driver_t * drv);
void unregister_clockevent(struct clockevent_t * ce);

int clockevent_set_event_handler(struct clockevent_t * ce, void (*handler)(struct clockevent_t *, void *), void * data);
int clockevent_set_event_next(struct clockevent_t * ce, ktime_t now, ktime_t expires);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_CLOCKEVENT_H__ */
