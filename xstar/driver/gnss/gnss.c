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

#include <driver/gnss/gnss.h>

static ssize_t gnss_write_enable(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gnss_t * nav = (struct gnss_t *)kobj->priv;
	gnss_enable(nav);
	return size;
}

static ssize_t gnss_write_disable(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gnss_t * nav = (struct gnss_t *)kobj->priv;
	gnss_disable(nav);
	return size;
}

struct gnss_t * search_gnss(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_GNSS);
	if(!dev)
		return NULL;
	return (struct gnss_t *)dev->priv;
}

struct gnss_t * search_first_gnss(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_GNSS);
	if(!dev)
		return NULL;
	return (struct gnss_t *)dev->priv;
}

static inline void gnss_nmea_init(struct gnss_nmea_t * nmea)
{
	if(nmea)
	{
		xos_memset(nmea, 0, sizeof(struct gnss_nmea_t));
		nmea->signal = GNSS_SIGNAL_INVALID;
		nmea->fix = GNSS_FIX_NONE;
		nmea->used = 0;
		nmea->latitude = 39.9042;
		nmea->longitude = 116.4074;
		nmea->altitude = 43.5;
		nmea->speed = 0.0;
		nmea->track = 0.0;
		nmea->mtrack = 0.0;
		nmea->magvar = -6.5;
		nmea->utc.year = 1981;
		nmea->utc.month = 6;
		nmea->utc.day = 26;
		nmea->utc.hour = 0;
		nmea->utc.minute = 0;
		nmea->utc.second = 0;
		nmea->utc.millisecond = 0;
	}
}

struct device_t * register_gnss(struct gnss_t * nav, struct driver_t * drv)
{
	struct device_t * dev;

	if(!nav || !nav->name || !nav->read)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(nav->name);
	dev->type = DEVICE_TYPE_GNSS;
	dev->driver = drv;
	dev->priv = nav;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "enable", NULL, gnss_write_enable, nav);
	kobj_add_regular(dev->kobj, "disable", NULL, gnss_write_disable, nav);
	gnss_nmea_init(&nav->nmea);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_gnss(struct gnss_t * nav)
{
	struct device_t * dev;

	if(nav && nav->name)
	{
		dev = search_device(nav->name, DEVICE_TYPE_GNSS);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

void gnss_enable(struct gnss_t * nav)
{
	if(nav && nav->enable)
		nav->enable(nav);
}

void gnss_disable(struct gnss_t * nav)
{
	if(nav && nav->disable)
		nav->disable(nav);
}

static int hex_to_int(const char * start, const char * end)
{
	int len = end - start;
	int result = 0;

	for(; len > 0; len--, start++)
	{
		if(start >= end)
			return 0;
		int c = *start;
		if((c >= '0') && (c <= '9'))
			c = c - '0';
		else if((c >= 'A') && (c <= 'F'))
			c = c - 'A' + 10;
		else if((c >= 'a') && (c <= 'f'))
			c = c - 'a' + 10;
		else
			return 0;
		result = result * 16 + c;
	}
	return result;
}

static int str_to_int(const char * start, const char * end)
{
	int len = end - start;
	int result = 0;

	for(; len > 0; len--, start++)
	{
		if(start >= end)
			return 0;
		int c = *start - '0';
		if(c >= 10)
			return 0;
		result = result * 10 + c;
	}
	return result;
}

static double latitude_longitude_convert(const char * s)
{
	double v = xos_strtod(s, NULL);
	int d = (int)(floor(v) / 100);
	return d + (v - d * 100) / 60.0;
}

static inline void gnss_nmea_parse_gga(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 9)
	{
		if(nmea->tend[1] - nmea->tstart[1] >= 6)
		{
			char * p = nmea->tstart[1];
			nmea->utc.hour = str_to_int(p, p + 2);
			nmea->utc.minute = str_to_int(p + 2, p + 4);
			nmea->utc.second = str_to_int(p + 4, p + 6);
			switch(nmea->tend[1] - nmea->tstart[1])
			{
			case 8:
				nmea->utc.millisecond = str_to_int(p + 7, p + 8) * 100;
				break;
			case 9:
				nmea->utc.millisecond = str_to_int(p + 7, p + 9) * 10;
				break;
			case 10:
				nmea->utc.millisecond = str_to_int(p + 7, p + 10);
				break;
			default:
				nmea->utc.millisecond = 0;
				break;
			}
		}

		if(nmea->tend[2] - nmea->tstart[2] >= 6)
		{
			nmea->latitude = latitude_longitude_convert(nmea->tstart[2]);
			if(xos_toupper(*nmea->tstart[3]) == 'S')
				nmea->latitude = -nmea->latitude;
		}

		if(nmea->tend[4] - nmea->tstart[4] >= 6)
		{
			nmea->longitude  = latitude_longitude_convert(nmea->tstart[4]);
			if(xos_toupper(*nmea->tstart[5]) == 'W')
				nmea->longitude  = -nmea->longitude ;
		}

		if(nmea->tend[6] - nmea->tstart[6] > 0)
			nmea->signal = str_to_int(nmea->tstart[6], nmea->tend[6]);
		else
			nmea->signal = GNSS_SIGNAL_INVALID;

		if(nmea->tend[7] - nmea->tstart[7] > 0)
			nmea->used = str_to_int(nmea->tstart[7], nmea->tend[7]);
		else
			nmea->used = 0;

		if(nmea->tend[8] - nmea->tstart[8] > 0)
			nmea->precision.hdop = xos_strtod(nmea->tstart[8], NULL);

		if(nmea->tend[9] - nmea->tstart[9] > 0)
			nmea->altitude = xos_strtod(nmea->tstart[9], NULL);
	}
}

static inline void gnss_nmea_parse_rmc(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 9)
	{
		if(nmea->tend[1] - nmea->tstart[1] >= 6)
		{
			char * p = nmea->tstart[1];
			nmea->utc.hour = str_to_int(p, p + 2);
			nmea->utc.minute = str_to_int(p + 2, p + 4);
			nmea->utc.second = str_to_int(p + 4, p + 6);
			switch(nmea->tend[1] - nmea->tstart[1])
			{
			case 8:
				nmea->utc.millisecond = str_to_int(p + 7, p + 8) * 100;
				break;
			case 9:
				nmea->utc.millisecond = str_to_int(p + 7, p + 9) * 10;
				break;
			case 10:
				nmea->utc.millisecond = str_to_int(p + 7, p + 10);
				break;
			default:
				nmea->utc.millisecond = 0;
				break;
			}
		}

		if(nmea->tend[3] - nmea->tstart[3] >= 6)
		{
			nmea->latitude = latitude_longitude_convert(nmea->tstart[3]);
			if(xos_toupper(*nmea->tstart[4]) == 'S')
				nmea->latitude = -nmea->latitude;
		}

		if(nmea->tend[5] - nmea->tstart[5] >= 6)
		{
			nmea->longitude  = latitude_longitude_convert(nmea->tstart[5]);
			if(xos_toupper(*nmea->tstart[6]) == 'W')
				nmea->longitude  = -nmea->longitude ;
		}

		if(nmea->tend[7] - nmea->tstart[7] > 0)
			nmea->speed = xos_strtod(nmea->tstart[7], NULL) * 1.852;

		if(nmea->tend[8] - nmea->tstart[8] > 0)
			nmea->track = xos_strtod(nmea->tstart[8], NULL);

		if(nmea->tend[9] - nmea->tstart[9] >= 6)
		{
			char * p = nmea->tstart[9];
			nmea->utc.day = str_to_int(p, p + 2);
			nmea->utc.month = str_to_int(p + 2, p + 4);
			nmea->utc.year = str_to_int(p + 4, p + 6) + 2000;
		}

		if(nmea->tsize > 11)
		{
			if(nmea->tend[10] - nmea->tstart[10] > 0)
			{
				nmea->magvar = xos_strtod(nmea->tstart[10], NULL);
				if(xos_toupper(*nmea->tstart[11]) == 'W')
					nmea->magvar = -nmea->magvar ;
			}
		}
	}
}

static inline void gnss_nmea_parse_vtg(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 7)
	{
		if(nmea->tend[1] - nmea->tstart[1] > 0)
			nmea->track = xos_strtod(nmea->tstart[1], NULL);

		if(nmea->tend[3] - nmea->tstart[3] > 0)
			nmea->mtrack = xos_strtod(nmea->tstart[3], NULL);

		if(nmea->tend[7] - nmea->tstart[7] > 0)
			nmea->speed = xos_strtod(nmea->tstart[7], NULL);
		else if(nmea->tend[5] - nmea->tstart[5] > 0)
			nmea->speed = xos_strtod(nmea->tstart[5], NULL) * 1.852;
	}
}

static inline void gnss_nmea_parse_gsa(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 17)
	{
		if(nmea->tend[2] - nmea->tstart[2] > 0)
			nmea->fix = str_to_int(nmea->tstart[2], nmea->tend[2]);
		else
			nmea->fix = GNSS_FIX_NONE;

		if(nmea->tend[15] - nmea->tstart[15] > 0)
			nmea->precision.pdop = xos_strtod(nmea->tstart[15], NULL);

		if(nmea->tend[16] - nmea->tstart[16] > 0)
			nmea->precision.hdop = xos_strtod(nmea->tstart[16], NULL);

		if(nmea->tend[17] - nmea->tstart[17] > 0)
			nmea->precision.vdop = xos_strtod(nmea->tstart[17], NULL);
	}
}

static inline void gnss_nmea_parse_gpgsv(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 3)
	{
		int total = str_to_int(nmea->tstart[1], nmea->tend[1]);
		int n = str_to_int(nmea->tstart[2], nmea->tend[2]);
		int viewed = XMIN((int)str_to_int(nmea->tstart[3], nmea->tend[3]), (int)ARRAY_SIZE(nmea->satellite.gps.sv));
		int c = (n <= 1) ? 0 : (n - 1) * 4;
		for(int i = 0; (i < 4) && (c < viewed); i++)
		{
			if(nmea->tsize > (i * 4 + 7))
			{
				nmea->satellite.gps.sv[c].prn = str_to_int(nmea->tstart[i * 4 + 4], nmea->tend[i * 4 + 4]);
				nmea->satellite.gps.sv[c].elevation = str_to_int(nmea->tstart[i * 4 + 5], nmea->tend[i * 4 + 5]);
				nmea->satellite.gps.sv[c].azimuth = str_to_int(nmea->tstart[i * 4 + 6], nmea->tend[i * 4 + 6]);
				nmea->satellite.gps.sv[c].snr = str_to_int(nmea->tstart[i * 4 + 7], nmea->tend[i * 4 + 7]);
			}
			else
			{
				nmea->satellite.gps.sv[c].prn = 0;
				nmea->satellite.gps.sv[c].elevation = 0;
				nmea->satellite.gps.sv[c].azimuth = 0;
				nmea->satellite.gps.sv[c].snr = 0;
			}
			c++;
		}
		if(n == total)
		{
			for(int i = viewed; i < ARRAY_SIZE(nmea->satellite.gps.sv); i++)
			{
				nmea->satellite.gps.sv[i].prn = 0;
				nmea->satellite.gps.sv[i].elevation = 0;
				nmea->satellite.gps.sv[i].azimuth = 0;
				nmea->satellite.gps.sv[i].snr = 0;
			}
			nmea->satellite.gps.n = viewed;
		}
	}
}

static inline void gnss_nmea_parse_bdgsv(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 3)
	{
		int total = str_to_int(nmea->tstart[1], nmea->tend[1]);
		int n = str_to_int(nmea->tstart[2], nmea->tend[2]);
		int viewed = XMIN((int)str_to_int(nmea->tstart[3], nmea->tend[3]), (int)ARRAY_SIZE(nmea->satellite.beidou.sv));
		int c = (n <= 1) ? 0 : (n - 1) * 4;
		for(int i = 0; (i < 4) && (c < viewed); i++)
		{
			if(nmea->tsize > (i * 4 + 7))
			{
				nmea->satellite.beidou.sv[c].prn = str_to_int(nmea->tstart[i * 4 + 4], nmea->tend[i * 4 + 4]);
				nmea->satellite.beidou.sv[c].elevation = str_to_int(nmea->tstart[i * 4 + 5], nmea->tend[i * 4 + 5]);
				nmea->satellite.beidou.sv[c].azimuth = str_to_int(nmea->tstart[i * 4 + 6], nmea->tend[i * 4 + 6]);
				nmea->satellite.beidou.sv[c].snr = str_to_int(nmea->tstart[i * 4 + 7], nmea->tend[i * 4 + 7]);
			}
			else
			{
				nmea->satellite.beidou.sv[c].prn = 0;
				nmea->satellite.beidou.sv[c].elevation = 0;
				nmea->satellite.beidou.sv[c].azimuth = 0;
				nmea->satellite.beidou.sv[c].snr = 0;
			}
			c++;
		}
		if(n == total)
		{
			for(int i = viewed; i < ARRAY_SIZE(nmea->satellite.beidou.sv); i++)
			{
				nmea->satellite.beidou.sv[i].prn = 0;
				nmea->satellite.beidou.sv[i].elevation = 0;
				nmea->satellite.beidou.sv[i].azimuth = 0;
				nmea->satellite.beidou.sv[i].snr = 0;
			}
			nmea->satellite.beidou.n = viewed;
		}
	}
}

static inline void gnss_nmea_parse_glgsv(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 3)
	{
		int total = str_to_int(nmea->tstart[1], nmea->tend[1]);
		int n = str_to_int(nmea->tstart[2], nmea->tend[2]);
		int viewed = XMIN((int)str_to_int(nmea->tstart[3], nmea->tend[3]), (int)ARRAY_SIZE(nmea->satellite.glonass.sv));
		int c = (n <= 1) ? 0 : (n - 1) * 4;
		for(int i = 0; (i < 4) && (c < viewed); i++)
		{
			if(nmea->tsize > (i * 4 + 7))
			{
				nmea->satellite.glonass.sv[c].prn = str_to_int(nmea->tstart[i * 4 + 4], nmea->tend[i * 4 + 4]);
				nmea->satellite.glonass.sv[c].elevation = str_to_int(nmea->tstart[i * 4 + 5], nmea->tend[i * 4 + 5]);
				nmea->satellite.glonass.sv[c].azimuth = str_to_int(nmea->tstart[i * 4 + 6], nmea->tend[i * 4 + 6]);
				nmea->satellite.glonass.sv[c].snr = str_to_int(nmea->tstart[i * 4 + 7], nmea->tend[i * 4 + 7]);
			}
			else
			{
				nmea->satellite.glonass.sv[c].prn = 0;
				nmea->satellite.glonass.sv[c].elevation = 0;
				nmea->satellite.glonass.sv[c].azimuth = 0;
				nmea->satellite.glonass.sv[c].snr = 0;
			}
			c++;
		}
		if(n == total)
		{
			for(int i = viewed; i < ARRAY_SIZE(nmea->satellite.glonass.sv); i++)
			{
				nmea->satellite.glonass.sv[i].prn = 0;
				nmea->satellite.glonass.sv[i].elevation = 0;
				nmea->satellite.glonass.sv[i].azimuth = 0;
				nmea->satellite.glonass.sv[i].snr = 0;
			}
			nmea->satellite.glonass.n = viewed;
		}
	}
}

static inline void gnss_nmea_parse_gagsv(struct gnss_nmea_t * nmea)
{
	if(nmea->tsize > 3)
	{
		int total = str_to_int(nmea->tstart[1], nmea->tend[1]);
		int n = str_to_int(nmea->tstart[2], nmea->tend[2]);
		int viewed = XMIN((int)str_to_int(nmea->tstart[3], nmea->tend[3]), (int)ARRAY_SIZE(nmea->satellite.galileo.sv));
		int c = (n <= 1) ? 0 : (n - 1) * 4;
		for(int i = 0; (i < 4) && (c < viewed); i++)
		{
			if(nmea->tsize > (i * 4 + 7))
			{
				nmea->satellite.galileo.sv[c].prn = str_to_int(nmea->tstart[i * 4 + 4], nmea->tend[i * 4 + 4]);
				nmea->satellite.galileo.sv[c].elevation = str_to_int(nmea->tstart[i * 4 + 5], nmea->tend[i * 4 + 5]);
				nmea->satellite.galileo.sv[c].azimuth = str_to_int(nmea->tstart[i * 4 + 6], nmea->tend[i * 4 + 6]);
				nmea->satellite.galileo.sv[c].snr = str_to_int(nmea->tstart[i * 4 + 7], nmea->tend[i * 4 + 7]);
			}
			else
			{
				nmea->satellite.galileo.sv[c].prn = 0;
				nmea->satellite.galileo.sv[c].elevation = 0;
				nmea->satellite.galileo.sv[c].azimuth = 0;
				nmea->satellite.galileo.sv[c].snr = 0;
			}
			c++;
		}
		if(n == total)
		{
			for(int i = viewed; i < ARRAY_SIZE(nmea->satellite.galileo.sv); i++)
			{
				nmea->satellite.galileo.sv[i].prn = 0;
				nmea->satellite.galileo.sv[i].elevation = 0;
				nmea->satellite.galileo.sv[i].azimuth = 0;
				nmea->satellite.galileo.sv[i].snr = 0;
			}
			nmea->satellite.galileo.n = viewed;
		}
	}
}

struct gnss_nmea_t * gnss_refresh(struct gnss_t * nav)
{
	if(nav)
	{
		struct gnss_nmea_t * nmea = &nav->nmea;
		char buf[256];
		int flag = 0;
		int n;
		while((n = nav->read(nav, buf, sizeof(buf))) > 0)
		{
			for(int idx = 0; idx < n; idx++)
			{
				char ch = buf[idx];
				switch(ch)
				{
				case '$':
					nmea->bindex = 0;
					nmea->dindex = 0;
					nmea->tsize = 0;
					break;

				case ',':
				case '*':
				case '\r':
					if(nmea->tsize < ARRAY_SIZE(nmea->tstart))
					{
						nmea->tstart[nmea->tsize] = &nmea->buffer[nmea->bindex - nmea->dindex];
						nmea->tend[nmea->tsize] = &nmea->buffer[nmea->bindex];
						nmea->tsize++;
					}
					if(nmea->bindex < sizeof(nmea->buffer))
					{
						nmea->buffer[nmea->bindex] = ch;
						nmea->bindex++;
					}
					nmea->dindex = 0;
					break;

				case '\n':
					if(nmea->tsize > 2)
					{
						unsigned char cksum = 0;
						for(int i = 0; i < nmea->tend[nmea->tsize - 2] - nmea->tstart[0]; i++)
							cksum ^= nmea->buffer[i];
						if(cksum == hex_to_int(nmea->tstart[nmea->tsize - 1], nmea->tend[nmea->tsize - 1]))
						{
							for(int i = 0; i < nmea->tsize; i++)
								*nmea->tend[i] = 0;
							switch(shash(nmea->tstart[0]))
							{
							case 0x0d30e1ab: /* "GPGGA" */
							case 0x0ccfd29a: /* "BDGGA" */
							case 0x0d2eb027: /* "GLGGA" */
							case 0x0d28a7fc: /* "GAGGA" */
							case 0x0d2fc8e9: /* "GNGGA" */
								gnss_nmea_parse_gga(nmea);
								break;

							case 0x0d31113e: /* "GPRMC" */
							case 0x0cd0022d: /* "BDRMC" */
							case 0x0d2edfba: /* "GLRMC" */
							case 0x0d28d78f: /* "GARMC" */
							case 0x0d2ff87c: /* "GNRMC" */
								gnss_nmea_parse_rmc(nmea);
								break;

							case 0x0d31232d: /* "GPVTG" */
							case 0x0cd0141c: /* "BDVTG" */
							case 0x0d2ef1a9: /* "GLVTG" */
							case 0x0d28e97e: /* "GAVTG" */
							case 0x0d300a6b: /* "GNVTG" */
								gnss_nmea_parse_vtg(nmea);
								break;

							case 0x0d30e337: /* "GPGSA" */
							case 0x0ccfd426: /* "BDGSA" */
							case 0x0d2eb1b3: /* "GLGSA" */
							case 0x0d28a988: /* "GAGSA" */
							case 0x0d2fca75: /* "GNGSA" */
								gnss_nmea_parse_gsa(nmea);
								break;

							case 0x0d30e34c: /* "GPGSV" */
								gnss_nmea_parse_gpgsv(nmea);
								break;

							case 0x0ccfd43b: /* "BDGSV" */
								gnss_nmea_parse_bdgsv(nmea);
								break;

							case 0x0d2eb1c8: /* "GLGSV" */
								gnss_nmea_parse_glgsv(nmea);
								break;

							case 0x0d28a99d: /* "GAGSV" */
								gnss_nmea_parse_gagsv(nmea);
								break;

							default:
								break;
							}
							flag = 1;
						}
					}
					nmea->bindex = 0;
					nmea->dindex = 0;
					nmea->tsize = 0;
					break;

				default:
					if(nmea->bindex < sizeof(nmea->buffer))
					{
						nmea->buffer[nmea->bindex] = ch;
						nmea->bindex++;
						nmea->dindex++;
					}
					break;
				}
			}
			if(n < sizeof(buf))
				break;
		}
		if(flag)
			return nmea;
	}
	return NULL;
}
