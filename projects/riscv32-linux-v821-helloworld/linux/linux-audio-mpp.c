#include <mpi_ao.h>
#include <mpi_ai.h>
#include <linux/linux.h>

/*
 * Playback
 */
struct linux_audio_playback_context_t {
	int rate;
	int channel;
};

void * linux_audio_playback_start(int rate, int channel)
{
	struct linux_audio_playback_context_t * ctx;

	ctx = linux_mem_malloc(sizeof(struct linux_audio_playback_context_t));
	if(!ctx)
		return NULL;

	ctx->rate = rate;
	ctx->channel = channel;
	AW_MPI_AO_CreateChn(0, 0);
	AW_MPI_AO_StartChn(0, 0);

	return ctx;
}

int linux_audio_playback_write(void * context, float * samples, int nsample)
{
	struct linux_audio_playback_context_t * ctx = (struct linux_audio_playback_context_t *)context;
	int16_t buffer[480];

	if(ctx)
	{
		int cnt = 0;
		while(nsample > 0)
		{
			int n = nsample > 480 ? 480 : nsample;
			for(int i = 0; i < n; i++)
				buffer[i] = (int16_t)(samples[i] * 32767.0f);
			if(n > 0)
			{
				AUDIO_FRAME_S frame;
				frame.mBitwidth = AUDIO_BIT_WIDTH_16;
				frame.mSoundmode = (ctx->channel == 2) ? AUDIO_SOUND_MODE_STEREO : AUDIO_SOUND_MODE_MONO;
				frame.mpAddr = (void *)&buffer[0];
				frame.mTimeStamp = 0;
				frame.mSeq = 0;
				frame.mLen = n << 1;
				frame.mId = 0;
				frame.mSamplerate = ctx->rate;
				frame.nBufSize = n << 1;
				AW_MPI_AO_SendFrameSync(0, 0, &frame);
			}
			samples += n;
			nsample -= n;
			cnt += n;
		}
		return cnt;
	}
	return 0;
}

void linux_audio_playback_stop(void * context)
{
	struct linux_audio_playback_context_t * ctx = (struct linux_audio_playback_context_t *)context;

	if(ctx)
	{
		AW_MPI_AO_StopChn(0, 0);
		AW_MPI_AO_DestroyChn(0, 0);
		linux_mem_free(ctx);
	}
}

void linux_audio_playback_set_volume(int vol)
{
	int value = XMAP((int)vol, 0, 1000, 0, 75);
	AW_MPI_AO_SetDevVolume(0, XCLAMP(value, 0, 100));
}

int linux_audio_playback_get_volume(void)
{
	int vol;

	AW_MPI_AO_GetDevVolume(0, (int *)&vol);
	int value = XMAP((int)vol, 0, 75, 0, 1000);
	return XCLAMP(value, 0, 1000);
}

/*
 * Capture
 */
struct linux_audio_capture_context_t {
	int rate;
	int channel;
	int start;
	struct fifo_t * fifo;
	struct thread_t * thread;
};
void (*linux_audio_capture_callback)(unsigned char * buf, unsigned int len) = NULL;

static ERRORTYPE capture_callback_func(void * cookie, AUDIO_DEV nAudioDevId, AudioDevEventType eEventType, int nPara0, void * pEventData)
{
	if(eEventType == AudioDevEvent_RawCapData)
	{
		AUDIO_FRAME_S * pframe = (AUDIO_FRAME_S *)pEventData;
		if(pframe && pframe->mpAddr && (pframe->mLen > 0))
		{
			if(linux_audio_capture_callback)
				linux_audio_capture_callback(pframe->mpAddr, pframe->mLen);
		}
	}
	return SUCCESS;
}

static void capture_thread_func(void * data)
{
	struct linux_audio_capture_context_t * ctx = (struct linux_audio_capture_context_t *)data;

	while(ctx->start)
	{
		AUDIO_FRAME_S frame;
		ERRORTYPE ret = AW_MPI_AI_GetFrame(0, 0, &frame, NULL, -1);
		if(ret == SUCCESS)
		{
			fifo_put(ctx->fifo, (unsigned char *)frame.mpAddr, frame.mLen);
			AW_MPI_AI_ReleaseFrame(0, 0, &frame, NULL);
		}
		else
			linux_thread_sleep(5 * 1000000ULL);
	}
}

void * linux_audio_capture_start(int rate, int channel)
{
	struct linux_audio_capture_context_t * ctx;

	ctx = linux_mem_malloc(sizeof(struct linux_audio_capture_context_t));
	if(!ctx)
		return NULL;

	ctx->rate = rate;
	ctx->channel = channel;
	ctx->start = 1;
	ctx->fifo = fifo_alloc(ctx->rate * ctx->channel * 2 * 120 / 1000);
	{
		AIO_ATTR_S mAIOAttr;
		memset(&mAIOAttr, 0, sizeof(AIO_ATTR_S));
		mAIOAttr.enSamplerate = ctx->rate;
		mAIOAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
		mAIOAttr.enSoundmode = (ctx->channel == 2) ? AUDIO_SOUND_MODE_STEREO : AUDIO_SOUND_MODE_MONO;
		mAIOAttr.mMicNum = 1;
		mAIOAttr.mChnCnt = 1;
		mAIOAttr.ai_aec_en = 1;
		mAIOAttr.aec_delay_ms = 0;
		mAIOAttr.mAecNlpMode = 1;
		mAIOAttr.mbBypassAec = 0;
		mAIOAttr.ai_ans_en = 1;
		mAIOAttr.ai_ans_mode = 3;
		mAIOAttr.ai_agc_en = 1;
		if(mAIOAttr.ai_agc_en)
		{
			mAIOAttr.ai_agc_float_cfg.fTargetDb = -6;
			mAIOAttr.ai_agc_float_cfg.fMaxGainDb = 20;
		}
		AW_MPI_AI_SetPubAttr(0, &mAIOAttr);
		AI_CHN_ATTR_S mAIChnAttr;
		xos_memset(&mAIChnAttr, 0, sizeof(AI_CHN_ATTR_S));
		mAIChnAttr.nFrameSize = 1920;
		AW_MPI_AI_CreateChn(0, 0, &mAIChnAttr);
		AW_MPI_AI_EnableChn(0, 0);
		AW_MPI_AI_RegisterDevCallback(0, (void *)ctx, capture_callback_func);
		AW_MPI_AI_EnableDevSendCaptureFrame(0, 1);
	}
	ctx->thread = linux_thread_create(NULL, capture_thread_func, ctx, 0);

	return ctx;
}

int linux_audio_capture_read(void * context, float * samples, int nsample)
{
	struct linux_audio_capture_context_t * ctx = (struct linux_audio_capture_context_t *)context;
	int16_t buffer[480];

	if(ctx)
	{
		int cnt = 0;
		while(nsample > 0)
		{
			int n = nsample > 480 ? 480 : nsample;
			int l = fifo_get(ctx->fifo, (unsigned char *)&buffer[0], (n << 1)) >> 1;
			if(l > 0)
			{
				for(int i = 0; i < l; i++)
					samples[i] = (float)buffer[i] / 32768.0f;
				samples += l;
				nsample -= l;
				cnt += l;
			}
			else
				break;
		}
		return cnt;
	}
	return 0;
}

void linux_audio_capture_stop(void * context)
{
	struct linux_audio_capture_context_t * ctx = (struct linux_audio_capture_context_t *)context;

	if(ctx)
	{
		ctx->start = 0;
		linux_thread_destroy(ctx->thread);
		fifo_free(ctx->fifo);
		AW_MPI_AI_DisableChn(0, 0);
		AW_MPI_AI_ResetChn(0, 0);
		AW_MPI_AI_DestroyChn(0, 0);
		linux_mem_free(ctx);
	}
}

void linux_audio_capture_set_volume(int vol)
{
	AW_MPI_AI_SetDevVolume(0, vol / 10);
}

int linux_audio_capture_get_volume(void)
{
	int vol;

	AW_MPI_AI_GetDevVolume(0, (int *)&vol);
	return (vol * 10);
}
