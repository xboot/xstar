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

#include <xstar.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    aplay -i=<source> -o=<sink> [-ie=<json>] [-oe=<json>] [-l]\r\n");
	shell_printf("    source: file:<path>\r\n");
	shell_printf("            capture:<device>,<rate>,<ch>\r\n");
	shell_printf("            tone:<waveform>,<rate>,<ch>,<freq>[,<ms>]\r\n");
	shell_printf("            noise:<rate>,<ch>\r\n");
	shell_printf("    sink:   playback:<device>,<rate>,<ch>\r\n");
	shell_printf("            amplitude:<period_ms>\r\n");
	shell_printf("            spectrum:<period_ms>\r\n");
	shell_printf("            vad:<start_hz>,<end_hz>\r\n");
	shell_printf("            afsk:<mark_hz>,<space_hz>,<rate>,<bitrate>\r\n");
}

static void aplay_cb_audiosink_amplitude(void * pdat, void * sdat)
{
    struct audio_sink_t * sink = (struct audio_sink_t *)pdat;
    float amplitude;

    audio_sink_ioctl(sink, "audio-sink-get-amplitude", &amplitude);
    shell_printf("[aplay] amplitude %.2f dB\r\n", 20.0f * log10f(amplitude + 1e-10f));
}

static void aplay_cb_audiosink_vad_active(void * pdat, void * sdat)
{
    shell_printf("[aplay] vad active\r\n");
}

static void aplay_cb_audiosink_vad_unactive(void * pdat, void * sdat)
{
    shell_printf("[aplay] vad inactive\r\n");
}

static struct audio_source_t * parse_source(const char * spec)
{
	char * dup = xos_strdup(spec);
	if(dup)
	{
		struct audio_source_t * source = NULL;
		char * p = dup;
		char * type = xos_strsep(&p, ":");
		if(type)
		{
			if(!xos_strcmp(type, "file") && p)
			{
				source = audio_source_alloc_from_xfs(shell_getxfs(), p);
			}
			else if(!xos_strcmp(type, "capture") && p)
			{
				char * device = xos_strsep(&p, ",");
				char * s_rate = xos_strsep(&p, ",");
				char * s_ch = xos_strsep(&p, ",");
				if(device && s_rate && s_ch)
				{
					int rate = xos_strtol(s_rate, NULL, 0);
					int ch = xos_strtol(s_ch, NULL, 0);
					source = audio_source_alloc_from_capture(device, rate, ch);
				}
			}
			else if(!xos_strcmp(type, "tone") && p)
			{
				char * waveform = xos_strsep(&p, ",");
				char * s_rate = xos_strsep(&p, ",");
				char * s_ch = xos_strsep(&p, ",");
				char * s_freq = xos_strsep(&p, ",");
				char * s_ms = xos_strsep(&p, ",");
				if(waveform && s_rate && s_ch && s_freq)
				{
					int rate = xos_strtol(s_rate, NULL, 0);
					int ch = xos_strtol(s_ch, NULL, 0);
					int freq = xos_strtol(s_freq, NULL, 0);
					int ms = s_ms ? xos_strtol(s_ms, NULL, 0) : 0;
					source = audio_source_alloc_tone(waveform, rate, ch, freq, ms);
				}
			}
			else if(!xos_strcmp(type, "noise") && p)
			{
				char * s_rate = xos_strsep(&p, ",");
				char * s_ch = xos_strsep(&p, ",");
				if(s_rate && s_ch)
				{
					int rate = xos_strtol(s_rate, NULL, 0);
					int ch = xos_strtol(s_ch, NULL, 0);
					source = audio_source_alloc_noise(rate, ch);
				}
			}
		}
		xos_mem_free(dup);
		return source;
	}
	return NULL;
}

static struct audio_sink_t * parse_sink(const char * spec)
{
	char * dup = xos_strdup(spec);
	if(dup)
	{
		struct audio_sink_t * sink = NULL;
		char * p = dup;
		char * type = xos_strsep(&p, ":");
		if(type)
		{
			if(!xos_strcmp(type, "playback") && p)
			{
				char * device = xos_strsep(&p, ",");
				char * s_rate = xos_strsep(&p, ",");
				char * s_ch = xos_strsep(&p, ",");
				if(device && s_rate && s_ch)
				{
					int rate = xos_strtol(s_rate, NULL, 0);
					int ch = xos_strtol(s_ch, NULL, 0);
					sink = audio_sink_alloc_from_playback(device, rate, ch);
				}
			}
			else if(!xos_strcmp(type, "amplitude") && p)
			{
				int period = xos_strtol(p, NULL, 0);
				sink = audio_sink_alloc_amplitude(period);
			}
			else if(!xos_strcmp(type, "spectrum") && p)
			{
				int period = xos_strtol(p, NULL, 0);
				sink = audio_sink_alloc_spectrum(period);
			}
			else if(!xos_strcmp(type, "vad") && p)
			{
				char * s_start = xos_strsep(&p, ",");
				char * s_end = xos_strsep(&p, ",");
				if(s_start && s_end)
				{
					int start = xos_strtol(s_start, NULL, 0);
					int end = xos_strtol(s_end, NULL, 0);
					sink = audio_sink_alloc_vad(start, end);
				}
			}
			else if(!xos_strcmp(type, "afsk") && p)
			{
				char * s_mark = xos_strsep(&p, ",");
				char * s_space = xos_strsep(&p, ",");
				char * s_rate = xos_strsep(&p, ",");
				char * s_bitrate = xos_strsep(&p, ",");
				if(s_mark && s_space && s_rate && s_bitrate)
				{
					int mark = xos_strtol(s_mark, NULL, 0);
					int space = xos_strtol(s_space, NULL, 0);
					int rate = xos_strtol(s_rate, NULL, 0);
					int bitrate = xos_strtol(s_bitrate, NULL, 0);
					sink = audio_sink_alloc_afsk(mark, space, rate, bitrate);
				}
			}
		}
		xos_mem_free(dup);
		return sink;
	}
	return NULL;
}

static int do_aplay(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char * []){ "-i", "-o", "-ie", "-oe", "-l", NULL }, 0, 0))
	{
		usage();
		return -1;
	}

	const char * ispec = sarg_get(&sarg, "-i", NULL);
	const char * ospec = sarg_get(&sarg, "-o", NULL);
	const char * ieffect = sarg_get(&sarg, "-ie", NULL);
	const char * oeffect = sarg_get(&sarg, "-oe", NULL);
	int loop = sarg_has(&sarg, "-l");

	if(!ispec || !ospec)
	{
		usage();
		return -1;
	}

	struct audio_source_t * source = parse_source(ispec);
	if(!source)
	{
		shell_printf("aplay: invalid source '%s'\r\n", ispec);
		return -1;
	}

	struct audio_sink_t * sink = parse_sink(ospec);
	if(!sink)
	{
		shell_printf("aplay: invalid sink '%s'\r\n", ospec);
		audio_source_free(source);
		return -1;
	}

	if(ieffect)
		audio_source_filter_apply(source, ieffect, xos_strlen(ieffect));

	if(oeffect)
		audio_sink_filter_apply(sink, oeffect, xos_strlen(oeffect));

	psub_subscribe("audiosink.amplitude", aplay_cb_audiosink_amplitude, NULL, 0);
	psub_subscribe("audiosink.vad.active", aplay_cb_audiosink_vad_active, NULL, 0);
	psub_subscribe("audiosink.vad.unactive", aplay_cb_audiosink_vad_unactive, NULL, 0);
	while(1)
	{
		struct audio_frame_t * af = audio_source_read(source);
		if(audio_frame_is_valid(af))
		{
			audio_sink_write(sink, af);
		}
		else
		{
			if(loop)
				audio_source_seek(source, 0);
			else
				break;
		}
		if(shell_ctrlc())
			break;
	}
	psub_unsubscribe("audiosink.amplitude", aplay_cb_audiosink_amplitude, NULL);
	psub_unsubscribe("audiosink.vad.active", aplay_cb_audiosink_vad_active, NULL);
	psub_unsubscribe("audiosink.vad.unactive", aplay_cb_audiosink_vad_unactive, NULL);

	audio_source_free(source);
	audio_sink_free(sink);

	return 0;
}

static struct command_t cmd_aplay = {
	.name	= "aplay",
	.desc	= "play audio source to sink with effects",
	.usage	= usage,
	.exec	= do_aplay,
};

static void aplay_cmd_init(void)
{
	register_command(&cmd_aplay);
}

static void aplay_cmd_exit(void)
{
	unregister_command(&cmd_aplay);
}

command_initcall(aplay_cmd_init);
command_exitcall(aplay_cmd_exit);
