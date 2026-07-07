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

#include <driver/audio/playback.h>

static ssize_t audio_playback_read_volume(struct kobj_t * kobj, void * buf, size_t size)
{
	struct audio_playback_t * playback = (struct audio_playback_t *)kobj->priv;
	return xos_sprintf(buf, "%d", audio_playback_get_volume(playback));
}

static ssize_t audio_playback_write_volume(struct kobj_t * kobj, void * buf, size_t size)
{
	struct audio_playback_t * playback = (struct audio_playback_t *)kobj->priv;
	audio_playback_set_volume(playback, xos_strtol(buf, NULL, 0));
	return size;
}

struct audio_playback_t * search_audio_playback(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_AUDIOPLAYBACK);
	if(!dev)
		return NULL;
	return (struct audio_playback_t *)dev->priv;
}

struct audio_playback_t * search_first_audio_playback(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_AUDIOPLAYBACK);
	if(!dev)
		return NULL;
	return (struct audio_playback_t *)dev->priv;
}

struct device_t * register_audio_playback(struct audio_playback_t * playback, struct driver_t * drv)
{
	struct device_t * dev;

	if(!playback || !playback->name || !playback->start || !playback->write || !playback->stop)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(playback->name);
	dev->type = DEVICE_TYPE_AUDIOPLAYBACK;
	dev->driver = drv;
	dev->priv = playback;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "volume", audio_playback_read_volume, audio_playback_write_volume, playback);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_audio_playback(struct audio_playback_t * playback)
{
	struct device_t * dev;

	if(playback && playback->name)
	{
		dev = search_device(playback->name, DEVICE_TYPE_AUDIOPLAYBACK);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int audio_playback_start(struct audio_playback_t * playback, int rate, int channel)
{
	if(playback)
		return playback->start(playback, rate, XMAX(channel, 1));
	return 0;
}

int audio_playback_write(struct audio_playback_t * playback, float * samples, int nsample)
{
	if(playback && samples && (nsample > 0))
		return playback->write(playback, samples, nsample);
	return 0;
}

int audio_playback_stop(struct audio_playback_t * playback)
{
	if(playback)
		return playback->stop(playback);
	return 0;
}

int audio_playback_ioctl(struct audio_playback_t * playback, const char * cmd, void * arg)
{
	if(playback && playback->ioctl)
		return playback->ioctl(playback, cmd, arg);
	return -1;
}

int audio_playback_get_volume(struct audio_playback_t * playback)
{
	int vol;

	if(audio_playback_ioctl(playback, "audio-playback-get-volume", &vol) >= 0)
		return vol;
	return 0;
}

void audio_playback_set_volume(struct audio_playback_t * playback, int vol)
{
	if(vol < 0)
		vol = 0;
	else if(vol > 1000)
		vol = 1000;
	audio_playback_ioctl(playback, "audio-playback-set-volume", &vol);
}
