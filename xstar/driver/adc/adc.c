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

#include <driver/adc/adc.h>

static ssize_t adc_read_vreference(struct kobj_t * kobj, void * buf, size_t size)
{
	struct adc_t * adc = (struct adc_t *)kobj->priv;
	return xos_sprintf(buf, "%Ld.%06LdV", adc->vreference / (uint64_t)(1000 * 1000), adc->vreference % (uint64_t)(1000 * 1000));
}

static ssize_t adc_read_resolution(struct kobj_t * kobj, void * buf, size_t size)
{
	struct adc_t * adc = (struct adc_t *)kobj->priv;
	return xos_sprintf(buf, "%d", adc->resolution);
}

static ssize_t adc_read_nchannel(struct kobj_t * kobj, void * buf, size_t size)
{
	struct adc_t * adc = (struct adc_t *)kobj->priv;
	return xos_sprintf(buf, "%d", adc->nchannel);
}

static ssize_t adc_read_raw_channel(struct kobj_t * kobj, void * buf, size_t size)
{
	struct adc_t * adc = (struct adc_t *)kobj->priv;
	int channel = xos_strtoul(kobj->name + xos_strlen("raw"), NULL, 0);
	return xos_sprintf(buf, "%d", adc_read_raw(adc, channel));
}

static ssize_t adc_read_voltage_channel(struct kobj_t * kobj, void * buf, size_t size)
{
	struct adc_t * adc = (struct adc_t *)kobj->priv;
	int channel = xos_strtoul(kobj->name + xos_strlen("voltage"), NULL, 0);
	int voltage = adc_read_voltage(adc, channel);
	return xos_sprintf(buf, "%Ld.%06LdV", voltage / (uint64_t)(1000 * 1000), voltage % (uint64_t)(1000 * 1000));
}

struct adc_t * search_adc(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_ADC);
	if(!dev)
		return NULL;
	return (struct adc_t *)dev->priv;
}

struct device_t * register_adc(struct adc_t * adc, struct driver_t * drv)
{
	struct device_t * dev;
	char buf[64];
	int i;

	if(!adc || !adc->name || (adc->resolution <= 0) || (adc->nchannel <= 0) || !adc->read)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(adc->name);
	dev->type = DEVICE_TYPE_ADC;
	dev->driver = drv;
	dev->priv = adc;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "vreference", adc_read_vreference, NULL, adc);
	kobj_add_regular(dev->kobj, "resolution", adc_read_resolution, NULL, adc);
	kobj_add_regular(dev->kobj, "nchannel", adc_read_nchannel, NULL, adc);
	for(i = 0; i< adc->nchannel; i++)
	{
		xos_sprintf(buf, "raw%d", i);
		kobj_add_regular(dev->kobj, buf, adc_read_raw_channel, NULL, adc);
	}
	for(i = 0; i< adc->nchannel; i++)
	{
		xos_sprintf(buf, "voltage%d", i);
		kobj_add_regular(dev->kobj, buf, adc_read_voltage_channel, NULL, adc);
	}

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_adc(struct adc_t * adc)
{
	struct device_t * dev;

	if(adc && adc->name)
	{
		dev = search_device(adc->name, DEVICE_TYPE_ADC);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int32_t adc_read_raw(struct adc_t * adc, int channel)
{
	if(adc && adc->read)
	{
		if(channel < 0)
			channel = 0;
		else if(channel > adc->nchannel - 1)
			channel = adc->nchannel - 1;
		return adc->read(adc, channel);
	}
	return 0;
}

int adc_read_voltage(struct adc_t * adc, int channel)
{
	if(adc && adc->read)
	{
		if(channel < 0)
			channel = 0;
		else if(channel > adc->nchannel - 1)
			channel = adc->nchannel - 1;
		return ((int64_t)adc->read(adc, channel) * adc->vreference) / ((1 << adc->resolution) - 1);
	}
	return 0;
}
