/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vad_sp.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "signal_processing/signal_processing_library.h"

// Allpass filter coefficients, upper and lower, in Q13.
// Upper: 0.64, Lower: 0.17
static const int16_t kAllPassCoefsQ13[2] = { 5243, 1392 };  // Q13
static const int16_t kSmoothingDown = 6553;  // 0.2 in Q15.
static const int16_t kSmoothingUp = 32439;   // 0.99 in Q15.

int WebRtcVad_Downsampling(const int16_t* signal, int16_t* out_signal,
                            int32_t* filter_state, int signal_length) {
  int16_t tmp16_1 = 0, tmp16_2 = 0;
  int32_t tmp32_1 = filter_state[0];
  int32_t tmp32_2 = filter_state[1];
  int n = 0;
  int half_length = signal_length >> 1;  // Downsampling by 2.

  // Filter even and odd samples separately, and then sum to get the downsampled
  // signal.
  for (n = 0; n < half_length; n++) {
    // All-pass filtering upper branch.
    tmp16_1 = (int16_t)((tmp32_1 >> 1) +
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[0], *signal, 14));
    *out_signal = tmp16_1;
    tmp32_1 = (int32_t)(*signal++) -
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[0], tmp16_1, 13);

    // All-pass filtering lower branch.
    tmp16_2 = (int16_t)((tmp32_2 >> 1) +
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[1], *signal, 14));
    *out_signal++ += tmp16_2;
    tmp32_2 = (int32_t)(*signal++) -
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[1], tmp16_2, 13);
  }
  filter_state[0] = tmp32_1;
  filter_state[1] = tmp32_2;

  return half_length;
}

// Finds the minimum value in a sliding window buffer.
// Used for noise estimation.
int16_t WebRtcVad_FindMinimum(VadInstT* inst, int16_t feature_value,
                               int channel_number) {
  int i = 0, j = 0;
  int position = -1;
  int current_median = 1600;
  int alpha = 0;
  int32_t tmp32 = 0;
  // Number of vectors stored.
  int16_t* age = &inst->age_vector[0];
  // Channel index for the feature.
  int16_t* smallest_values = &inst->low_value_vector[channel_number * 16];
  int16_t* age_for_channel = &inst->index_vector[channel_number * 16];

  assert(channel_number < kNumChannels);

  // Search buffer for |feature_value|.
  for (i = 0; i < 16; i++) {
    if (feature_value <= smallest_values[i]) {
      if (i == 0 || feature_value != smallest_values[i - 1]) {
        position = i;
      }
    }
  }

  // Update and shift the buffer.
  if (position == -1) {
    // |feature_value| is larger than all values. Insert at end.
    smallest_values[15] = feature_value;
    age_for_channel[15] = 1;
  } else {
    // Shift values.
    memmove(&smallest_values[position + 1], &smallest_values[position],
            sizeof(int16_t) * (15 - position));
    memmove(&age_for_channel[position + 1], &age_for_channel[position],
            sizeof(int16_t) * (15 - position));
    smallest_values[position] = feature_value;
    age_for_channel[position] = 1;
  }

  // Compute the median: use median value in the buffer as noise estimate.
  current_median = smallest_values[7];

  // Smooth the noise with |alpha|, depending on direction.
  if (inst->mean_value[channel_number] < current_median) {
    alpha = kSmoothingDown;  // 0.2 in Q15
  } else {
    alpha = kSmoothingUp;  // 0.99 in Q15
  }

  tmp32 = WEBRTC_SPL_MUL_16_16(alpha + 1, inst->mean_value[channel_number]);
  tmp32 += WEBRTC_SPL_MUL_16_16(WEBRTC_SPL_WORD16_MAX - alpha,
                                  (int16_t)current_median);
  tmp32 >>= 15;
  inst->mean_value[channel_number] = (int16_t)tmp32;

  return inst->mean_value[channel_number];
}
