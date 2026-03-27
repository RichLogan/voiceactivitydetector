/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vad_filterbank.h"

#include <stdint.h>
#include <string.h>

#include "signal_processing/signal_processing_library.h"

// TODO(bjornv): The Q-factor comments are somewhat hard to follow.
// Investigate if they can be made clearer.

// Coefficients used by WebRtcVad_HpOutput, Q14.
static const int16_t kHpZeroCoefs[3] = { 6631, -13262, 6631 };
static const int16_t kHpPoleCoefs[3] = { 16384, -7756, 5620 };

// Allpass filter coefficients, upper, Q15.
static const int16_t kAllPassCoefsQ15[2] = { 20972, 29691 };  // 0.64, 0.92

// Allpass filter coefficients, lower, Q15.
static const int16_t kAllPassCoefsQ13[2] = { 6549, 28124 };  // 0.20, 0.86

static void AllPassFilter(const int16_t* data_in, size_t data_length,
                          const int16_t* filter_coefficients, int16_t* filter_state,
                          int16_t* data_out) {
  // The filter can only cause one sample delay.
  int i;
  int16_t tmp16 = 0;
  int32_t tmp32 = 0;
  int32_t state32 = ((int32_t) filter_state[0] << 16);  // Q31

  for (i = 0; i < (int)data_length; i++) {
    tmp32 = state32 + WEBRTC_SPL_MUL_16_16(filter_coefficients[0], data_in[i]);
    tmp16 = (int16_t) (tmp32 >> 16);  // Q(-1) -> Q15
    data_out[i] = tmp16;
    state32 = ((int32_t) data_in[i] << 14) -
        WEBRTC_SPL_MUL_16_16(filter_coefficients[0], tmp16);
    state32 <<= 1;
  }
  filter_state[0] = (int16_t) (state32 >> 16);  // Q(-1)
}

// Splits |data_in| into |hp_data_out| and |lp_data_out| corresponding to
// an upper (high) and lower (low) frequency band. This is done by applying
// the upper and lower allpass filters, and then performing either an addition
// or a subtraction.
//
// |data_in|           : Input audio data to be filtered.
// |data_length|       : Length of input audio data.
// |upper_state|       : Filter state for the upper allpass filter.
// |lower_state|       : Filter state for the lower allpass filter.
// |hp_data_out|       : Output audio data of the upper half-band.
// |lp_data_out|       : Output audio data of the lower half-band.
static void SplitFilter(const int16_t* data_in, size_t data_length,
                        int16_t* upper_state, int16_t* lower_state,
                        int16_t* hp_data_out, int16_t* lp_data_out) {
  int i;
  int half_length = (int)data_length >> 1;  // Downsampling by 2.
  int16_t tmp_out;

  // All pass filter on even samples.
  AllPassFilter(&data_in[0], (size_t)half_length, kAllPassCoefsQ15, upper_state,
                hp_data_out);

  // All pass filter on odd samples.
  AllPassFilter(&data_in[1], (size_t)half_length, kAllPassCoefsQ13, lower_state,
                lp_data_out);

  // Make LP and HP signals.
  for (i = 0; i < half_length; i++) {
    tmp_out = hp_data_out[i];
    hp_data_out[i] = hp_data_out[i] - lp_data_out[i];
    lp_data_out[i] = tmp_out + lp_data_out[i];
  }
}

// Calculates the energy of |data_in| in dB, and also updates the
// |inst->upper_state| and |inst->lower_state|.
//
// - inst        [i/o] : State information of the VAD.
// - data_in     [i]   : Input audio data for feature extraction.
// - data_length [i]   : Audio data size in Frames.
// - offset      [i]   : Offset used to compensate for the downsampling.
// - features    [o]   : 10 * log10(energy) for the frequency band.
int16_t WebRtcVad_LogOfEnergy(VadInstT* inst, const int16_t* data_in,
                               size_t data_length, int16_t offset,
                               int16_t* features) {
  int16_t hp_out[240], lp_out[240];
  int16_t hp_out_s[240], lp_out_s[240];
  int16_t hp_out_ss[240], lp_out_ss[240];
  size_t half_size = data_length >> 1;
  size_t quarter_size = data_length >> 2;

  int16_t log2_energy, energy_s;
  int energy_scale = 0;
  int16_t energy;
  int32_t en_sq;
  int tot_rshifts;
  int i;

  // Split at 2000 Hz.
  SplitFilter(data_in, data_length, &inst->upper_state[0], &inst->lower_state[0],
              hp_out, lp_out);

  // For the upper band (2000-4000 Hz):
  // Split at 3000 Hz.
  SplitFilter(hp_out, half_size, &inst->upper_state[1], &inst->lower_state[1],
              hp_out_s, lp_out_s);

  // For the lower band (0-2000 Hz):
  // Split at 1000 Hz.
  SplitFilter(lp_out, half_size, &inst->upper_state[2], &inst->lower_state[2],
              hp_out_ss, lp_out_ss);

  // For the upper lower band (1000-2000 Hz):
  // Split at 1500 Hz (approximately).
  SplitFilter(hp_out_ss, quarter_size, &inst->upper_state[3], &inst->lower_state[3],
              hp_out, lp_out);

  // For the lower lower band (0-1000 Hz):
  // Split at 500 Hz.
  SplitFilter(lp_out_ss, quarter_size, &inst->upper_state[4], &inst->lower_state[4],
              hp_out_ss, lp_out_ss);

  // Compute energy for each of the 6 bands:
  // Band 0: lp_out_ss  (0-250 Hz)
  // Band 1: hp_out_ss  (250-500 Hz)
  // Band 2: lp_out     (500-1000 Hz)
  // Band 3: hp_out     (1000-2000 Hz)
  // Band 4: lp_out_s   (2000-3000 Hz)
  // Band 5: hp_out_s   (3000-4000 Hz)

  {
    int16_t* bands[6] = {lp_out_ss, hp_out_ss, lp_out, hp_out, lp_out_s, hp_out_s};
    size_t band_sizes[6];
    band_sizes[0] = band_sizes[1] = data_length >> 3;
    band_sizes[2] = band_sizes[3] = data_length >> 2;
    band_sizes[4] = band_sizes[5] = data_length >> 1;

    for (i = 0; i < kNumChannels; i++) {
      int scale = 0;
      int32_t band_energy = 0;
      size_t j;
      int16_t smax = WebRtcSpl_MaxAbsValueW16(bands[i], band_sizes[i]);
      // Normalize so that the max value is close to 2^14
      int norm_shift = WebRtcSpl_NormW16(smax) - 2;
      if (norm_shift < 0) norm_shift = 0;
      for (j = 0; j < band_sizes[i]; j++) {
        int32_t s = (int32_t)(bands[i][j] << norm_shift);
        band_energy += (s * s) >> 15;
        if (band_energy < 0) { band_energy = INT32_MAX; break; }
      }
      // band_energy is now in some Q-factor. Compute log2 and convert to dB.
      if (band_energy > 0) {
        int norm = WebRtcSpl_NormW32(band_energy);
        int32_t normalized = band_energy << norm;
        // log2(energy) = 31 - norm (integer part) + log2(normalized/2^31)
        // Use: log2(x) ~ (x - 1) for x near 1, but we'll use a table or linear approx
        // Approx: log2(band_energy) ~= 31 - norm
        log2_energy = (int16_t)((31 - norm - 2 * norm_shift) * 2048);
        // Convert log2 to log10: log10(x) = log2(x) / log2(10) = log2(x) * 0.30103
        // kLog2 = 2048, log10 factor = 0.30103, so in Q11:
        // features[i] = log2_energy * 617 / 2048  (617 ~ 0.30103 * 2048)
        features[i] = (int16_t)(((int32_t)log2_energy * 617) >> 11) + offset;
      } else {
        features[i] = offset;
      }
    }
  }

  return 0;
}
