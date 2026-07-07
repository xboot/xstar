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

#include <driver/audio/capture.h>

static ssize_t audio_capture_read_volume(struct kobj_t * kobj, void * buf, size_t size)
{
	struct audio_capture_t * capture = (struct audio_capture_t *)kobj->priv;
	return xos_sprintf(buf, "%d", audio_capture_get_volume(capture));
}

static ssize_t audio_capture_write_volume(struct kobj_t * kobj, void * buf, size_t size)
{
	struct audio_capture_t * capture = (struct audio_capture_t *)kobj->priv;
	audio_capture_set_volume(capture, xos_strtol(buf, NULL, 0));
	return size;
}

struct audio_capture_t * search_audio_capture(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_AUDIOCAPTURE);
	if(!dev)
		return NULL;
	return (struct audio_capture_t *)dev->priv;
}

struct audio_capture_t * search_first_audio_capture(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_AUDIOCAPTURE);
	if(!dev)
		return NULL;
	return (struct audio_capture_t *)dev->priv;
}

struct device_t * register_audio_capture(struct audio_capture_t * capture, struct driver_t * drv)
{
	struct device_t * dev;

	if(!capture || !capture->name || !capture->start || !capture->read || !capture->stop)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(capture->name);
	dev->type = DEVICE_TYPE_AUDIOCAPTURE;
	dev->driver = drv;
	dev->priv = capture;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "volume", audio_capture_read_volume, audio_capture_write_volume, capture);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_audio_capture(struct audio_capture_t * capture)
{
	struct device_t * dev;

	if(capture && capture->name)
	{
		dev = search_device(capture->name, DEVICE_TYPE_AUDIOCAPTURE);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int audio_capture_start(struct audio_capture_t * capture, int rate, int channel)
{
	if(capture)
		return capture->start(capture, rate, XMAX(channel, 1));
	return 0;
}

int audio_capture_read(struct audio_capture_t * capture, float * samples, int nsample)
{
	if(capture && samples && (nsample > 0))
		return capture->read(capture, samples, nsample);
	return 0;
}

int audio_capture_stop(struct audio_capture_t * capture)
{
	if(capture)
		return capture->stop(capture);
	return 0;
}

int audio_capture_ioctl(struct audio_capture_t * capture, const char * cmd, void * arg)
{
	if(capture && capture->ioctl)
		return capture->ioctl(capture, cmd, arg);
	return -1;
}

int audio_capture_get_volume(struct audio_capture_t * capture)
{
	int vol;

	if(audio_capture_ioctl(capture, "audio-capture-get-volume", &vol) >= 0)
		return vol;
	return 0;
}

void audio_capture_set_volume(struct audio_capture_t * capture, int vol)
{
	if(vol < 0)
		vol = 0;
	else if(vol > 1000)
		vol = 1000;
	audio_capture_ioctl(capture, "audio-capture-set-volume", &vol);
}
