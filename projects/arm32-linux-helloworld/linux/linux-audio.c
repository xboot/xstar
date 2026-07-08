#include <linux/linux.h>

struct linux_audio_context_t {
	snd_pcm_t * handle;
	snd_pcm_hw_params_t * params;
	snd_pcm_uframes_t frames;
	snd_pcm_format_t format;
	int channel;
};

/*
 * Playback
 */
void * linux_audio_playback_start(int rate, int channel)
{
	struct linux_audio_context_t * ctx;
	unsigned int val = rate;
	unsigned int buffer_time;
	unsigned int period_time;
	int dir = 0;

	ctx = malloc(sizeof(struct linux_audio_context_t));
	if(!ctx)
		return NULL;
	memset(ctx, 0, sizeof(struct linux_audio_context_t));

	if(snd_pcm_open(&ctx->handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{
		free(ctx);
		return NULL;
	}
	snd_pcm_hw_params_alloca(&ctx->params);
	if(snd_pcm_hw_params_any(ctx->handle, ctx->params) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_set_access(ctx->handle, ctx->params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{
		free(ctx);
		return NULL;
	}
	ctx->format = SND_PCM_FORMAT_FLOAT;
	if(snd_pcm_hw_params_set_format(ctx->handle, ctx->params, ctx->format) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_set_channels(ctx->handle, ctx->params, channel) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_set_rate_near(ctx->handle, ctx->params, &val, &dir) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_get_buffer_time_max(ctx->params, &buffer_time, 0) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(buffer_time > 100000)
		buffer_time = 100000;
	period_time = buffer_time / 4;
	if(snd_pcm_hw_params_set_period_time_near(ctx->handle, ctx->params, &period_time, 0) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_set_buffer_time_near(ctx->handle, ctx->params, &buffer_time, 0) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params(ctx->handle, ctx->params) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_get_period_size(ctx->params, &ctx->frames, &dir) < 0)
	{
		free(ctx);
		return NULL;
	}
	snd_pcm_sw_params_t * swparams;
	snd_pcm_sw_params_alloca(&swparams);
	if(snd_pcm_sw_params_current(ctx->handle, swparams) >= 0)
	{
		snd_pcm_sw_params_set_start_threshold(ctx->handle, swparams, ctx->frames * 2);
		snd_pcm_sw_params(ctx->handle, swparams);
	}
	ctx->channel = channel;
	return ctx;
}

int linux_audio_playback_write(void * context, float * samples, int nsample)
{
	struct linux_audio_context_t * ctx = (struct linux_audio_context_t *)context;

	if(ctx && ctx->handle)
	{
		int frames = XMIN((int)(nsample / ctx->channel), (int)ctx->frames);
		int ret = snd_pcm_writei(ctx->handle, samples, frames);
		if(ret > 0)
			return ret * ctx->channel;
		else if(ret == -EPIPE)
		{
			snd_pcm_prepare(ctx->handle);
			snd_pcm_start(ctx->handle);
		}
	}
	return 0;
}

void linux_audio_playback_stop(void * context)
{
	struct linux_audio_context_t * ctx = (struct linux_audio_context_t *)context;

	if(ctx)
	{
		if(ctx->handle)
		{
			snd_pcm_drop(ctx->handle);
			snd_pcm_close(ctx->handle);
		}
		free(ctx);
	}
}

void linux_audio_playback_set_volume(int vol)
{
	snd_mixer_t * handle;
	snd_mixer_selem_id_t * sid;
	snd_mixer_elem_t * elem;
	const char * card = "default";
	const char * name = "Master";
	long minvol, maxvol;

	if(snd_mixer_open(&handle, 0) == 0)
	{
		if(snd_mixer_attach(handle, card) == 0)
		{
			snd_mixer_selem_register(handle, NULL, NULL);
			snd_mixer_load(handle);
			snd_mixer_selem_id_alloca(&sid);
			snd_mixer_selem_id_set_index(sid, 0);
			snd_mixer_selem_id_set_name(sid, name);
			elem = snd_mixer_find_selem(handle, sid);
			if(elem)
			{
				snd_mixer_selem_get_playback_volume_range(elem, &minvol, &maxvol);
				snd_mixer_selem_set_playback_volume_all(elem, vol * (maxvol - minvol) / 1000);
			}
		}
		snd_mixer_close(handle);
	}
}

int linux_audio_playback_get_volume(void)
{
	snd_mixer_t * handle;
	snd_mixer_selem_id_t * sid;
	snd_mixer_elem_t * elem;
	const char * card = "default";
	const char * name = "Master";
	long minvol, maxvol;
	long vol = 0;

	if(snd_mixer_open(&handle, 0) == 0)
	{
		if(snd_mixer_attach(handle, card) == 0)
		{
			snd_mixer_selem_register(handle, NULL, NULL);
			snd_mixer_load(handle);
			snd_mixer_selem_id_alloca(&sid);
			snd_mixer_selem_id_set_index(sid, 0);
			snd_mixer_selem_id_set_name(sid, name);
			elem = snd_mixer_find_selem(handle, sid);
			if(elem)
			{
				snd_mixer_selem_get_playback_volume_range(elem, &minvol, &maxvol);
				snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
				vol = vol * 1000 / (maxvol - minvol);
				if(vol < 0)
					vol = 0;
				else if(vol > 1000)
					vol = 1000;
			}
		}
		snd_mixer_close(handle);
	}
	return (int)vol;
}

/*
 * Capture
 */
void * linux_audio_capture_start(int rate, int channel)
{
	struct linux_audio_context_t * ctx;
	unsigned int val = rate;
	int dir = 0;

	ctx = malloc(sizeof(struct linux_audio_context_t));
	if(!ctx)
		return NULL;
	memset(ctx, 0, sizeof(struct linux_audio_context_t));

	if(snd_pcm_open(&ctx->handle, "default", SND_PCM_STREAM_CAPTURE, 0) < 0)
	{
		free(ctx);
		return NULL;
	}
	snd_pcm_hw_params_alloca(&ctx->params);
	if(snd_pcm_hw_params_any(ctx->handle, ctx->params) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_set_access(ctx->handle, ctx->params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{
		free(ctx);
		return NULL;
	}
	ctx->format = SND_PCM_FORMAT_FLOAT;
	if(snd_pcm_hw_params_set_format(ctx->handle, ctx->params, ctx->format) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_set_channels(ctx->handle, ctx->params, channel) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_set_rate_near(ctx->handle, ctx->params, &val, &dir) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params(ctx->handle, ctx->params) < 0)
	{
		free(ctx);
		return NULL;
	}
	if(snd_pcm_hw_params_get_period_size(ctx->params, &ctx->frames, &dir) < 0)
	{
		free(ctx);
		return NULL;
	}
	ctx->channel = channel;
	return ctx;
}

int linux_audio_capture_read(void * context, float * samples, int nsample)
{
	struct linux_audio_context_t * ctx = (struct linux_audio_context_t *)context;

	if(ctx && ctx->handle)
	{
		int frames = XMIN((int)(nsample / ctx->channel), (int)ctx->frames);
		int ret = snd_pcm_readi(ctx->handle, samples, frames);
		if(ret > 0)
			return ret * ctx->channel;
		else if(ret == -EPIPE)
		{
			snd_pcm_prepare(ctx->handle);
			snd_pcm_start(ctx->handle);
		}
	}
	return 0;
}

void linux_audio_capture_stop(void * context)
{
	struct linux_audio_context_t * ctx = (struct linux_audio_context_t *)context;

	if(ctx)
	{
		if(ctx->handle)
		{
			snd_pcm_drop(ctx->handle);
			snd_pcm_close(ctx->handle);
		}
		free(ctx);
	}
}

void linux_audio_capture_set_volume(int vol)
{
	snd_mixer_t * handle;
	snd_mixer_selem_id_t * sid;
	snd_mixer_elem_t * elem;
	const char * card = "default";
	const char * name = "Capture";
	long minvol, maxvol;

	if(snd_mixer_open(&handle, 0) == 0)
	{
		if(snd_mixer_attach(handle, card) == 0)
		{
			snd_mixer_selem_register(handle, NULL, NULL);
			snd_mixer_load(handle);
			snd_mixer_selem_id_alloca(&sid);
			snd_mixer_selem_id_set_index(sid, 0);
			snd_mixer_selem_id_set_name(sid, name);
			elem = snd_mixer_find_selem(handle, sid);
			if(elem)
			{
				snd_mixer_selem_get_capture_volume_range(elem, &minvol, &maxvol);
				snd_mixer_selem_set_capture_volume_all(elem, vol * (maxvol - minvol) / 1000);
			}
		}
		snd_mixer_close(handle);
	}
}

int linux_audio_capture_get_volume(void)
{
	snd_mixer_t * handle;
	snd_mixer_selem_id_t * sid;
	snd_mixer_elem_t * elem;
	const char * card = "default";
	const char * name = "Capture";
	long minvol, maxvol;
	long vol;

	if(snd_mixer_open(&handle, 0) == 0)
	{
		if(snd_mixer_attach(handle, card) == 0)
		{
			snd_mixer_selem_register(handle, NULL, NULL);
			snd_mixer_load(handle);
			snd_mixer_selem_id_alloca(&sid);
			snd_mixer_selem_id_set_index(sid, 0);
			snd_mixer_selem_id_set_name(sid, name);
			elem = snd_mixer_find_selem(handle, sid);
			if(elem)
			{
				snd_mixer_selem_get_capture_volume_range(elem, &minvol, &maxvol);
				snd_mixer_selem_get_capture_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
				vol = vol * 1000 / (maxvol - minvol);
				if(vol < 0)
					vol = 0;
				else if(vol > 1000)
					vol = 1000;
			}
		}
		snd_mixer_close(handle);
	}
	return (int)vol;
}
