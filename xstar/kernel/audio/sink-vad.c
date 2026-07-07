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

#include <kernel/core/psub.h>
#include <kernel/audio/sink.h>

#define VAD_NUM_CHANNELS	(6)
#define VAD_NUM_GAUSSIANS	(2)
#define VAD_TABLE_SIZE		(VAD_NUM_CHANNELS * VAD_NUM_GAUSSIANS)
#define VAD_MIN_ENERGY		(10)

struct vad_ctx_t {
	int16_t noise_means[VAD_TABLE_SIZE];
	int16_t speech_means[VAD_TABLE_SIZE];
	int16_t noise_stds[VAD_TABLE_SIZE];
	int16_t speech_stds[VAD_TABLE_SIZE];
	int32_t frame_counter;
	int16_t over_hang;
	int16_t num_of_speech;
	int16_t index_vector[16 * VAD_NUM_CHANNELS];
	int16_t low_value_vector[16 * VAD_NUM_CHANNELS];
	int16_t mean_value[VAD_NUM_CHANNELS];
	int16_t upper_state[5];
	int16_t lower_state[5];
	int16_t hp_filter_state[4];
	int16_t over_hang_max_1[3];
	int16_t over_hang_max_2[3];
	int16_t individual[3];
	int16_t total[3];
};

static inline int spl_count_leading_zero_s32(uint32_t n)
{
	return (n == 0) ? 32 : __builtin_clz(n);
}

static inline int16_t spl_get_size_in_bits(uint32_t n)
{
	return 32 - spl_count_leading_zero_s32(n);
}

static inline int16_t spl_norm_w32(int32_t a)
{
	return (a == 0) ? 0 : spl_count_leading_zero_s32(a < 0 ? ~a : a) - 1;
}

static inline int16_t spl_norm_u32(uint32_t a)
{
	return (a == 0) ? 0 : spl_count_leading_zero_s32(a);
}

static inline int32_t spl_div_w32_w16(int32_t num, int16_t den)
{
	if(den != 0)
		return (int32_t)(num / den);
	else
		return (int32_t)0x7fffffff;
}

static inline int32_t overflowing_mul_s16_by_s32_to_s32(int16_t a, int32_t b)
{
	return a * b;
}

static int16_t spl_get_scaling_square(int16_t * in_vector, size_t in_vector_length, size_t times)
{
	int16_t nbits = spl_get_size_in_bits((uint32_t)times);
	size_t i;
	int16_t smax = -1;
	int16_t sabs;
	int16_t *sptr = in_vector;
	int16_t t;
	size_t looptimes = in_vector_length;

	for(i = looptimes; i > 0; i--)
	{
		sabs = (*sptr > 0 ? *sptr++ : -*sptr++);
		smax = (sabs > smax ? sabs : smax);
	}
	t = spl_norm_w32((int32_t)(smax) * (int32_t)(smax));
	if(smax == 0)
		return 0;
	else
		return (t > nbits) ? 0 : nbits - t;
}

static int32_t spl_energy(int16_t *vector, size_t vector_length, int *scale_factor)
{
	int32_t en = 0;
	int scaling = spl_get_scaling_square(vector, vector_length, vector_length);
	size_t looptimes = vector_length;
	int16_t *vectorptr = vector;

	for(size_t i = 0; i < looptimes; i++)
	{
		en += (*vectorptr * (*vectorptr)) >> scaling;
		vectorptr++;
	}
	*scale_factor = scaling;
	return en;
}

static const int16_t kSpectrumWeight[VAD_NUM_CHANNELS] = { 6, 8, 10, 12, 14, 16 };
static const int16_t kNoiseUpdateConst = 655;
static const int16_t kSpeechUpdateConst = 6554;
static const int16_t kBackEta = 154;
static const int16_t kMinimumDifference[VAD_NUM_CHANNELS] = { 544, 544, 576, 576, 576, 576 };
static const int16_t kMaximumSpeech[VAD_NUM_CHANNELS] = { 11392, 11392, 11520, 11520, 11520, 11520 };
static const int16_t kMinimumMean[VAD_NUM_GAUSSIANS] = { 640, 768 };
static const int16_t kMaximumNoise[VAD_NUM_CHANNELS] = { 9216, 9088, 8960, 8832, 8704, 8576 };
static const int16_t kNoiseDataWeights[VAD_TABLE_SIZE] = { 34, 62, 72, 66, 53, 25, 94, 66, 56, 62, 75, 103 };
static const int16_t kSpeechDataWeights[VAD_TABLE_SIZE] = { 48, 82, 45, 87, 50, 47, 80, 46, 83, 41, 78, 81 };
static const int16_t kNoiseDataMeans[VAD_TABLE_SIZE] = { 6738, 4892, 7065, 6715, 6771, 3369, 7646, 3863, 7820, 7266, 5020, 4362 };
static const int16_t kSpeechDataMeans[VAD_TABLE_SIZE] = { 8306, 10085, 10078, 11823, 11843, 6309, 9473, 9571, 10879, 7581, 8180, 7483 };
static const int16_t kNoiseDataStds[VAD_TABLE_SIZE] = { 378, 1064, 493, 582, 688, 593, 474, 697, 475, 688, 421, 455 };
static const int16_t kSpeechDataStds[VAD_TABLE_SIZE] = { 555, 505, 567, 524, 585, 1231, 509, 828, 492, 1540, 1079, 850 };
static const int16_t kMaxSpeechFrames = 6;
static const int16_t kMinStd = 384;

static const int16_t kHpZeroCoefs[3] = { 6631, -13262, 6631 };
static const int16_t kHpPoleCoefs[3] = { 16384, -7756, 5620 };
static const int16_t kAllPassCoefsQ15[2] = { 20972, 5571 };
static const int16_t kOffsetVector[6] = { 368, 368, 272, 176, 176, 176 };

static int32_t weighted_average(int16_t *data, int16_t offset, const int16_t *weights)
{
	int32_t weighted_average = 0;

	for(int k = 0; k < VAD_NUM_GAUSSIANS; k++)
	{
		data[k * VAD_NUM_CHANNELS] += offset;
		weighted_average += data[k * VAD_NUM_CHANNELS] * weights[k * VAD_NUM_CHANNELS];
	}
	return weighted_average;
}

static void high_pass_filter(const int16_t *data_in, size_t data_length, int16_t *filter_state, int16_t *data_out)
{
	const int16_t *in_ptr = data_in;
	int16_t *out_ptr = data_out;
	int32_t tmp32 = 0;

	for(size_t i = 0; i < data_length; i++)
	{
		tmp32 = kHpZeroCoefs[0] * *in_ptr;
		tmp32 += kHpZeroCoefs[1] * filter_state[0];
		tmp32 += kHpZeroCoefs[2] * filter_state[1];
		filter_state[1] = filter_state[0];
		filter_state[0] = *in_ptr++;

		tmp32 -= kHpPoleCoefs[1] * filter_state[2];
		tmp32 -= kHpPoleCoefs[2] * filter_state[3];
		filter_state[3] = filter_state[2];
		filter_state[2] = (int16_t)(tmp32 >> 14);
		*out_ptr++ = filter_state[2];
	}
}

static void all_pass_filter(const int16_t *data_in, size_t data_length, int16_t filter_coefficient, int16_t *filter_state, int16_t *data_out)
{
	int16_t tmp16 = 0;
	int32_t tmp32 = 0;
	int32_t state32 = ((int32_t)(*filter_state) * (1 << 16));

	for(size_t i = 0; i < data_length; i++)
	{
		tmp32 = state32 + filter_coefficient * *data_in;
		tmp16 = (int16_t)(tmp32 >> 16);
		*data_out++ = tmp16;
		state32 = (*data_in * (1 << 14)) - filter_coefficient * tmp16;
		state32 *= 2;
		data_in += 2;
	}
	*filter_state = (int16_t)(state32 >> 16);
}

static void split_filter(const int16_t *data_in, size_t data_length, int16_t *upper_state, int16_t *lower_state, int16_t *hp_data_out, int16_t *lp_data_out)
{
	size_t half_length = data_length >> 1;
	int16_t tmp_out;

	all_pass_filter(&data_in[0], half_length, kAllPassCoefsQ15[0], upper_state, hp_data_out);
	all_pass_filter(&data_in[1], half_length, kAllPassCoefsQ15[1], lower_state, lp_data_out);
	for(size_t i = 0; i < half_length; i++)
	{
		tmp_out = *hp_data_out;
		*hp_data_out++ -= *lp_data_out;
		*lp_data_out++ += tmp_out;
	}
}

static void log_of_energy(const int16_t *data_in, size_t data_length, int16_t offset, int16_t *total_energy, int16_t *log_energy)
{
	int tot_rshifts = 0;
	uint32_t energy = 0;

	energy = (uint32_t)spl_energy((int16_t*)data_in, data_length, &tot_rshifts);
	if(energy != 0)
	{
		int normalizing_rshifts = 17 - spl_norm_u32(energy);
		int16_t log2_energy = 14336;

		tot_rshifts += normalizing_rshifts;
		if(normalizing_rshifts < 0)
			energy <<= -normalizing_rshifts;
		else
			energy >>= normalizing_rshifts;
		log2_energy += (int16_t)((energy & 0x00003FFF) >> 4);
		*log_energy = (int16_t)(((24660 * log2_energy) >> 19) + ((tot_rshifts * 24660) >> 9));
		if(*log_energy < 0)
			*log_energy = 0;
	}
	else
	{
		*log_energy = offset;
		return;
	}
	*log_energy += offset;

	if(*total_energy <= VAD_MIN_ENERGY)
	{
		if(tot_rshifts >= 0)
			*total_energy += VAD_MIN_ENERGY + 1;
		else
			*total_energy += (int16_t)(energy >> -tot_rshifts);  // Q0.
	}
}

static int16_t vad_calculate_features(struct vad_ctx_t * ctx, const int16_t * data_in, size_t data_length, int16_t * features)
{
	int16_t total_energy = 0;
	int16_t hp_120[120], lp_120[120];
	int16_t hp_60[60], lp_60[60];
	const size_t half_data_length = data_length >> 1;
	size_t length = half_data_length;
	int frequency_band = 0;
	const int16_t *in_ptr = data_in;
	int16_t *hp_out_ptr = hp_120;
	int16_t *lp_out_ptr = lp_120;

	split_filter(in_ptr, data_length, &ctx->upper_state[frequency_band], &ctx->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);
	frequency_band = 1;
	in_ptr = hp_120;
	hp_out_ptr = hp_60;
	lp_out_ptr = lp_60;
	split_filter(in_ptr, length, &ctx->upper_state[frequency_band], &ctx->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);
	length >>= 1;
	log_of_energy(hp_60, length, kOffsetVector[5], &total_energy, &features[5]);
	log_of_energy(lp_60, length, kOffsetVector[4], &total_energy, &features[4]);
	frequency_band = 2;
	in_ptr = lp_120;
	hp_out_ptr = hp_60;
	lp_out_ptr = lp_60;
	length = half_data_length;
	split_filter(in_ptr, length, &ctx->upper_state[frequency_band], &ctx->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);
	length >>= 1;
	log_of_energy(hp_60, length, kOffsetVector[3], &total_energy, &features[3]);
	frequency_band = 3;
	in_ptr = lp_60;
	hp_out_ptr = hp_120;
	lp_out_ptr = lp_120;
	split_filter(in_ptr, length, &ctx->upper_state[frequency_band], &ctx->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);
	length >>= 1;
	log_of_energy(hp_120, length, kOffsetVector[2], &total_energy, &features[2]);
	frequency_band = 4;
	in_ptr = lp_120;
	hp_out_ptr = hp_60;
	lp_out_ptr = lp_60;
	split_filter(in_ptr, length, &ctx->upper_state[frequency_band], &ctx->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);
	length >>= 1;
	log_of_energy(hp_60, length, kOffsetVector[1], &total_energy, &features[1]);
	high_pass_filter(lp_60, length, ctx->hp_filter_state, hp_120);
	log_of_energy(hp_120, length, kOffsetVector[0], &total_energy, &features[0]);

	return total_energy;
}

static int32_t vad_gaussian_probability(int16_t input, int16_t mean, int16_t std, int16_t *delta)
{
	int16_t tmp16, inv_std, inv_std2, exp_value = 0;
	int32_t tmp32;

	tmp32 = (int32_t)131072 + (int32_t)(std >> 1);
	inv_std = (int16_t)spl_div_w32_w16(tmp32, std);
	tmp16 = (inv_std >> 2);
	inv_std2 = (int16_t)((tmp16 * tmp16) >> 2);
	tmp16 = (input << 3);
	tmp16 = tmp16 - mean;
	*delta = (int16_t)((inv_std2 * tmp16) >> 10);
	tmp32 = (*delta * tmp16) >> 9;
	if(tmp32 < 22005)
	{
		tmp16 = (int16_t)((5909 * tmp32) >> 12);
		tmp16 = -tmp16;
		exp_value = (0x0400 | (tmp16 & 0x03FF));
		tmp16 ^= 0xFFFF;
		tmp16 >>= 10;
		tmp16 += 1;
		exp_value >>= tmp16;
	}
	return inv_std * exp_value;
}

static int16_t vad_find_minimum(struct vad_ctx_t *ctx, int16_t feature_value, int channel)
{
	int i = 0, j = 0;
	int position = -1;
	const int offset = (channel << 4);
	int16_t current_median = 1600;
	int16_t alpha = 0;
	int32_t tmp32 = 0;
	int16_t *age = &ctx->index_vector[offset];
	int16_t *smallest_values = &ctx->low_value_vector[offset];

	for(i = 0; i < 16; i++)
	{
		if(age[i] != 100)
			age[i]++;
		else
		{
			for(j = i; j < 15; j++)
			{
				smallest_values[j] = smallest_values[j + 1];
				age[j] = age[j + 1];
			}
			age[15] = 101;
			smallest_values[15] = 10000;
		}
	}

	if(feature_value < smallest_values[7])
	{
		if(feature_value < smallest_values[3])
		{
			if(feature_value < smallest_values[1])
			{
				if(feature_value < smallest_values[0])
					position = 0;
				else
					position = 1;
			}
			else if(feature_value < smallest_values[2])
				position = 2;
			else
				position = 3;
		}
		else if(feature_value < smallest_values[5])
		{
			if(feature_value < smallest_values[4])
				position = 4;
			else
				position = 5;
		}
		else if(feature_value < smallest_values[6])
			position = 6;
		else
			position = 7;
	}
	else if(feature_value < smallest_values[15])
	{
		if(feature_value < smallest_values[11])
		{
			if(feature_value < smallest_values[9])
			{
				if(feature_value < smallest_values[8])
					position = 8;
				else
					position = 9;
			}
			else if(feature_value < smallest_values[10])
				position = 10;
			else
				position = 11;
		}
		else if(feature_value < smallest_values[13])
		{
			if(feature_value < smallest_values[12])
				position = 12;
			else
				position = 13;
		}
		else if(feature_value < smallest_values[14])
			position = 14;
		else
			position = 15;
	}
	if(position > -1)
	{
		for(i = 15; i > position; i--)
		{
			smallest_values[i] = smallest_values[i - 1];
			age[i] = age[i - 1];
		}
		smallest_values[position] = feature_value;
		age[position] = 1;
	}
	if(ctx->frame_counter > 2)
		current_median = smallest_values[2];
	else if(ctx->frame_counter > 0)
		current_median = smallest_values[0];

	if(ctx->frame_counter > 0)
	{
		if(current_median < ctx->mean_value[channel])
			alpha = 6553;
		else
			alpha = 32439;
	}
	tmp32 = (alpha + 1) * ctx->mean_value[channel];
	tmp32 += (32767 - alpha) * current_median;
	tmp32 += 16384;
	ctx->mean_value[channel] = (int16_t)(tmp32 >> 15);

	return ctx->mean_value[channel];
}

static int16_t gmm_probability(struct vad_ctx_t * ctx, int16_t * features, int16_t total_power, size_t frame_length)
{
	int channel, k;
	int16_t feature_minimum;
	int16_t h0, h1;
	int16_t log_likelihood_ratio;
	int16_t vadflag = 0;
	int16_t shifts_h0, shifts_h1;
	int16_t tmp_s16, tmp1_s16, tmp2_s16;
	int16_t diff;
	int gaussian;
	int16_t nmk, nmk2, nmk3, smk, smk2, nsk, ssk;
	int16_t delt, ndelt;
	int16_t maxspe, maxmu;
	int16_t deltaN[VAD_TABLE_SIZE], deltaS[VAD_TABLE_SIZE];
	int16_t ngprvec[VAD_TABLE_SIZE] = { 0 };
	int16_t sgprvec[VAD_TABLE_SIZE] = { 0 };
	int32_t h0_test, h1_test;
	int32_t tmp1_s32, tmp2_s32;
	int32_t sum_log_likelihood_ratios = 0;
	int32_t noise_global_mean, speech_global_mean;
	int32_t noise_probability[VAD_NUM_GAUSSIANS], speech_probability[VAD_NUM_GAUSSIANS];
	int16_t overhead1, overhead2, individualTest, totalTest;

	if(frame_length == 80)
	{
		overhead1 = ctx->over_hang_max_1[0];
		overhead2 = ctx->over_hang_max_2[0];
		individualTest = ctx->individual[0];
		totalTest = ctx->total[0];
	}
	else if(frame_length == 160)
	{
		overhead1 = ctx->over_hang_max_1[1];
		overhead2 = ctx->over_hang_max_2[1];
		individualTest = ctx->individual[1];
		totalTest = ctx->total[1];
	}
	else
	{
		overhead1 = ctx->over_hang_max_1[2];
		overhead2 = ctx->over_hang_max_2[2];
		individualTest = ctx->individual[2];
		totalTest = ctx->total[2];
	}

	if(total_power > VAD_MIN_ENERGY)
	{
		for(channel = 0; channel < VAD_NUM_CHANNELS; channel++)
		{
			h0_test = 0;
			h1_test = 0;
			for(k = 0; k < VAD_NUM_GAUSSIANS; k++)
			{
				gaussian = channel + k * VAD_NUM_CHANNELS;
				tmp1_s32 = vad_gaussian_probability(features[channel], ctx->noise_means[gaussian], ctx->noise_stds[gaussian], &deltaN[gaussian]);
				noise_probability[k] = kNoiseDataWeights[gaussian] * tmp1_s32;
				h0_test += noise_probability[k];

				tmp1_s32 = vad_gaussian_probability(features[channel], ctx->speech_means[gaussian], ctx->speech_stds[gaussian], &deltaS[gaussian]);
				speech_probability[k] = kSpeechDataWeights[gaussian] * tmp1_s32;
				h1_test += speech_probability[k];
			}

			shifts_h0 = spl_norm_w32(h0_test);
			shifts_h1 = spl_norm_w32(h1_test);
			if(h0_test == 0)
				shifts_h0 = 31;
			if(h1_test == 0)
				shifts_h1 = 31;
			log_likelihood_ratio = shifts_h0 - shifts_h1;
			sum_log_likelihood_ratios += (int32_t)(log_likelihood_ratio * kSpectrumWeight[channel]);
			if((log_likelihood_ratio * 4) > individualTest)
				vadflag = 1;
			h0 = (int16_t)(h0_test >> 12);
			if(h0 > 0)
			{
				tmp1_s32 = (noise_probability[0] & 0xFFFFF000) << 2;
				ngprvec[channel] = (int16_t)spl_div_w32_w16(tmp1_s32, h0);
				ngprvec[channel + VAD_NUM_CHANNELS] = 16384 - ngprvec[channel];
			}
			else
				ngprvec[channel] = 16384;
			h1 = (int16_t)(h1_test >> 12);
			if(h1 > 0)
			{
				tmp1_s32 = (speech_probability[0] & 0xFFFFF000) << 2;
				sgprvec[channel] = (int16_t)spl_div_w32_w16(tmp1_s32, h1);
				sgprvec[channel + VAD_NUM_CHANNELS] = 16384 - sgprvec[channel];
			}
		}

		vadflag |= (sum_log_likelihood_ratios >= totalTest);
		maxspe = 12800;
		for(channel = 0; channel < VAD_NUM_CHANNELS; channel++)
		{
			feature_minimum = vad_find_minimum(ctx, features[channel], channel);
			noise_global_mean = weighted_average(&ctx->noise_means[channel], 0, &kNoiseDataWeights[channel]);
			tmp1_s16 = (int16_t)(noise_global_mean >> 6);

			for(k = 0; k < VAD_NUM_GAUSSIANS; k++)
			{
				gaussian = channel + k * VAD_NUM_CHANNELS;
				nmk = ctx->noise_means[gaussian];
				smk = ctx->speech_means[gaussian];
				nsk = ctx->noise_stds[gaussian];
				ssk = ctx->speech_stds[gaussian];
				nmk2 = nmk;
				if(!vadflag)
				{
					delt = (int16_t)((ngprvec[gaussian] * deltaN[gaussian]) >> 11);
					nmk2 = nmk + (int16_t)((delt * kNoiseUpdateConst) >> 22);
				}
				ndelt = (feature_minimum << 4) - tmp1_s16;
				nmk3 = nmk2 + (int16_t)((ndelt * kBackEta) >> 9);
				tmp_s16 = (int16_t)((k + 5) << 7);
				if(nmk3 < tmp_s16)
					nmk3 = tmp_s16;
				tmp_s16 = (int16_t)((72 + k - channel) << 7);
				if(nmk3 > tmp_s16)
					nmk3 = tmp_s16;
				ctx->noise_means[gaussian] = nmk3;

				if(vadflag)
				{
					delt = (int16_t)((sgprvec[gaussian] * deltaS[gaussian]) >> 11);
					tmp_s16 = (int16_t)((delt * kSpeechUpdateConst) >> 21);
					smk2 = smk + ((tmp_s16 + 1) >> 1);

					maxmu = maxspe + 640;
					if(smk2 < kMinimumMean[k])
						smk2 = kMinimumMean[k];
					if(smk2 > maxmu)
						smk2 = maxmu;
					ctx->speech_means[gaussian] = smk2;
					tmp_s16 = ((smk + 4) >> 3);
					tmp_s16 = features[channel] - tmp_s16;
					tmp1_s32 = (deltaS[gaussian] * tmp_s16) >> 3;
					tmp2_s32 = tmp1_s32 - 4096;
					tmp_s16 = sgprvec[gaussian] >> 2;
					tmp1_s32 = tmp_s16 * tmp2_s32;
					tmp2_s32 = tmp1_s32 >> 4;
					if(tmp2_s32 > 0)
						tmp_s16 = (int16_t)spl_div_w32_w16(tmp2_s32, ssk * 10);
					else
					{
						tmp_s16 = (int16_t)spl_div_w32_w16(-tmp2_s32, ssk * 10);
						tmp_s16 = -tmp_s16;
					}
					tmp_s16 += 128;
					ssk += (tmp_s16 >> 8);
					if(ssk < kMinStd)
						ssk = kMinStd;
					ctx->speech_stds[gaussian] = ssk;
				}
				else
				{
					tmp_s16 = features[channel] - (nmk >> 3);
					tmp1_s32 = (deltaN[gaussian] * tmp_s16) >> 3;
					tmp1_s32 -= 4096;
					tmp_s16 = (ngprvec[gaussian] + 2) >> 2;
					tmp2_s32 = overflowing_mul_s16_by_s32_to_s32(tmp_s16, tmp1_s32);
					tmp1_s32 = tmp2_s32 >> 14;
					if(tmp1_s32 > 0)
						tmp_s16 = (int16_t)spl_div_w32_w16(tmp1_s32, nsk);
					else
					{
						tmp_s16 = (int16_t)spl_div_w32_w16(-tmp1_s32, nsk);
						tmp_s16 = -tmp_s16;
					}
					tmp_s16 += 32;
					nsk += tmp_s16 >> 6;
					if(nsk < kMinStd)
						nsk = kMinStd;
					ctx->noise_stds[gaussian] = nsk;
				}
			}
			noise_global_mean = weighted_average(&ctx->noise_means[channel], 0, &kNoiseDataWeights[channel]);
			speech_global_mean = weighted_average(&ctx->speech_means[channel], 0, &kSpeechDataWeights[channel]);
			diff = (int16_t)(speech_global_mean >> 9) - (int16_t)(noise_global_mean >> 9);
			if(diff < kMinimumDifference[channel])
			{
				tmp_s16 = kMinimumDifference[channel] - diff;
				tmp1_s16 = (int16_t)((13 * tmp_s16) >> 2);
				tmp2_s16 = (int16_t)((3 * tmp_s16) >> 2);
				speech_global_mean = weighted_average(&ctx->speech_means[channel], tmp1_s16, &kSpeechDataWeights[channel]);
				noise_global_mean = weighted_average(&ctx->noise_means[channel], -tmp2_s16, &kNoiseDataWeights[channel]);
			}
			maxspe = kMaximumSpeech[channel];
			tmp2_s16 = (int16_t)(speech_global_mean >> 7);
			if(tmp2_s16 > maxspe)
			{
				tmp2_s16 -= maxspe;
				for(k = 0; k < VAD_NUM_GAUSSIANS; k++)
					ctx->speech_means[channel + k * VAD_NUM_CHANNELS] -= tmp2_s16;
			}
			tmp2_s16 = (int16_t)(noise_global_mean >> 7);
			if(tmp2_s16 > kMaximumNoise[channel])
			{
				tmp2_s16 -= kMaximumNoise[channel];
				for(k = 0; k < VAD_NUM_GAUSSIANS; k++)
					ctx->noise_means[channel + k * VAD_NUM_CHANNELS] -= tmp2_s16;
			}
		}
		ctx->frame_counter++;
	}
	if(!vadflag)
	{
		if(ctx->over_hang > 0)
		{
			vadflag = 2 + ctx->over_hang;
			ctx->over_hang--;
		}
		ctx->num_of_speech = 0;
	}
	else
	{
		ctx->num_of_speech++;
		if(ctx->num_of_speech > kMaxSpeechFrames)
		{
			ctx->num_of_speech = kMaxSpeechFrames;
			ctx->over_hang = overhead2;
		}
		else
			ctx->over_hang = overhead1;
	}
	return vadflag;
}

static void vad_setmode(struct vad_ctx_t * ctx, int mode)
{
	switch(mode)
	{
	case 0:	/* Quality */
		ctx->over_hang_max_1[0] = 8;
		ctx->over_hang_max_1[1] = 4;
		ctx->over_hang_max_1[2] = 3;
		ctx->over_hang_max_2[0] = 14;
		ctx->over_hang_max_2[1] = 7;
		ctx->over_hang_max_2[2] = 5;
		ctx->individual[0] = 24;
		ctx->individual[1] = 21;
		ctx->individual[2] = 24;
		ctx->total[0] = 57;
		ctx->total[1] = 48;
		ctx->total[2] = 57;
		break;
	case 1:	/* Low bitrate */
		ctx->over_hang_max_1[0] = 8;
		ctx->over_hang_max_1[1] = 4;
		ctx->over_hang_max_1[2] = 3;
		ctx->over_hang_max_2[0] = 14;
		ctx->over_hang_max_2[1] = 7;
		ctx->over_hang_max_2[2] = 5;
		ctx->individual[0] = 37;
		ctx->individual[1] = 32;
		ctx->individual[2] = 37;
		ctx->total[0] = 100;
		ctx->total[1] = 80;
		ctx->total[2] = 100;
		break;
	case 2:	/* Aggressive */
		ctx->over_hang_max_1[0] = 6;
		ctx->over_hang_max_1[1] = 3;
		ctx->over_hang_max_1[2] = 2;
		ctx->over_hang_max_2[0] = 9;
		ctx->over_hang_max_2[1] = 5;
		ctx->over_hang_max_2[2] = 3;
		ctx->individual[0] = 82;
		ctx->individual[1] = 78;
		ctx->individual[2] = 82;
		ctx->total[0] = 285;
		ctx->total[1] = 260;
		ctx->total[2] = 285;
		break;
	case 3:	/* Very aggressive */
		ctx->over_hang_max_1[0] = 6;
		ctx->over_hang_max_1[1] = 3;
		ctx->over_hang_max_1[2] = 2;
		ctx->over_hang_max_2[0] = 9;
		ctx->over_hang_max_2[1] = 5;
		ctx->over_hang_max_2[2] = 3;
		ctx->individual[0] = 94;
		ctx->individual[1] = 94;
		ctx->individual[2] = 94;
		ctx->total[0] = 1100;
		ctx->total[1] = 1050;
		ctx->total[2] = 1100;
		break;
	default:
		break;
	}
}

static void vad_init(struct vad_ctx_t * ctx, int mode)
{
	ctx->frame_counter = 0;
	ctx->over_hang = 0;
	ctx->num_of_speech = 0;
	for(int i = 0; i < VAD_TABLE_SIZE; i++)
	{
		ctx->noise_means[i] = kNoiseDataMeans[i];
		ctx->speech_means[i] = kSpeechDataMeans[i];
		ctx->noise_stds[i] = kNoiseDataStds[i];
		ctx->speech_stds[i] = kSpeechDataStds[i];
	}
	for(int i = 0; i < 16 * VAD_NUM_CHANNELS; i++)
	{
		ctx->low_value_vector[i] = 10000;
		ctx->index_vector[i] = 0;
	}
	xos_memset(ctx->upper_state, 0, sizeof(ctx->upper_state));
	xos_memset(ctx->lower_state, 0, sizeof(ctx->lower_state));
	xos_memset(ctx->hp_filter_state, 0, sizeof(ctx->hp_filter_state));
	for(int i = 0; i < VAD_NUM_CHANNELS; i++)
		ctx->mean_value[i] = 1600;
	vad_setmode(ctx, mode);
}

static struct vad_ctx_t * vadctx_alloc(void)
{
	struct vad_ctx_t * ctx = (struct vad_ctx_t *)xos_mem_malloc(sizeof(struct vad_ctx_t));

	if(ctx)
		vad_init(ctx, 0);
	return ctx;
}

static void vadctx_free(struct vad_ctx_t * ctx)
{
	if(ctx)
		xos_mem_free(ctx);
}

static int vad_process(struct vad_ctx_t * ctx, int16_t * frames, int length)
{
	int16_t feature_vector[VAD_NUM_CHANNELS];
	int16_t total_power = vad_calculate_features(ctx, frames, length, feature_vector);
	return (gmm_probability(ctx, feature_vector, total_power, length) > 0) ? 1 : 0;
}

struct audio_sink_vad_pdata_t {
	struct vad_ctx_t * ctx;
	int16_t buffer[160];
	int bufidx;
	int himaxcnt, hicnt;
	int lomaxcnt, locnt;
	int status;
	struct audio_filter_t * filter;
};

static void audio_sink_vad_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
	struct audio_sink_vad_pdata_t * pdat = (struct audio_sink_vad_pdata_t *)s->priv;
	struct audio_frame_t * output = audio_filter_process(pdat->filter, af);
	float * po = output->samples;

	for(int n = 0; n < output->frames; n++)
	{
		pdat->buffer[pdat->bufidx++] = (int16_t)(po[n] * 32767.0f);
		if(pdat->bufidx >= 160)
		{
			pdat->bufidx = 0;
			if(vad_process(pdat->ctx, pdat->buffer, 160))
			{
				pdat->locnt = 0;
				if(!pdat->status)
				{
					if(++pdat->hicnt >= pdat->himaxcnt)
					{
						pdat->status = 1;
						psub_publish("audiosink.vad.active", (void *)s);
					}
				}
			}
			else
			{
				pdat->hicnt = 0;
				if(pdat->status)
				{
					if(++pdat->locnt >= pdat->lomaxcnt)
					{
						pdat->status = 0;
						psub_publish("audiosink.vad.unactive", (void *)s);
					}
				}
			}
		}
	}
}

static int audio_sink_vad_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
	return -1;
}

static void audio_sink_vad_destroy(struct audio_sink_t * s)
{
	struct audio_sink_vad_pdata_t * pdat = (struct audio_sink_vad_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->ctx)
			vadctx_free(pdat->ctx);
		if(pdat->filter)
			audio_filter_free(pdat->filter);
		xos_mem_free(pdat);
	}
}

struct audio_sink_t * audio_sink_alloc_vad(int start, int end)
{
	struct audio_sink_vad_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_sink_vad_pdata_t));
	if(!pdat)
		return NULL;

	pdat->ctx = vadctx_alloc();
	pdat->bufidx = 0;
	pdat->himaxcnt = (((start > 0) ? start : 1) + 20 - 1) / 20;
	pdat->hicnt = 0;
	pdat->lomaxcnt = (((end > 0) ? end : 1) + 20 - 1) / 20;
	pdat->locnt = 0;
	pdat->status = 0;

	char json[256];
	int length = xos_sprintf(json, "{\"duplicate\":{},\"mono\":{},\"resample\":{\"rate\":8000}}");
	pdat->filter = audio_filter_alloc(json, length);
	if(!pdat->filter)
	{
		vadctx_free(pdat->ctx);
		xos_mem_free(pdat);
		return NULL;
	}

	struct audio_sink_t * s = audio_sink_alloc();
	if(!s)
	{
		vadctx_free(pdat->ctx);
		audio_filter_free(pdat->filter);
		xos_mem_free(pdat);
		return NULL;
	}
	s->write = audio_sink_vad_write;
	s->ioctl = audio_sink_vad_ioctl;
	s->destroy = audio_sink_vad_destroy;
	s->priv = pdat;

	return s;
}
