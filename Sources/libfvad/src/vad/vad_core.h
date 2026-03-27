/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VAD_VAD_CORE_H_
#define VAD_VAD_CORE_H_

#include <stdint.h>
#include <stddef.h>

#include "signal_processing/signal_processing_library.h"
#include "signal_processing/resample_48khz.h"

enum { kNumChannels = 6 };  // Number of frequency bands (named channels).
enum { kNumGaussians = 2 };  // Number of Gaussians per channel in the GMM.
enum { kTableSize = kNumChannels * kNumGaussians };
enum { kMinEnergy = 10 };  // Minimum energy required to trigger audio response.

typedef struct VadInstT_ {
    int vad;  // Unspecified state.
    int32_t downsampling_filter_states[4];
    WebRtcSpl_State48khzTo8khz state_48_to_8;
    int16_t noise_means[kTableSize];
    int16_t speech_means[kTableSize];
    int16_t noise_stds[kTableSize];
    int16_t speech_stds[kTableSize];
    // TODO(bjornv): Change to |frame_count|.
    int32_t frame_counter;
    int16_t over_hang;  // Over-hang counter in frames.
    int16_t num_of_speech;
    // TODO(bjornv): Change to |age_vector|.
    int16_t age_vector[16];
    int16_t index_vector[kNumChannels * 16];
    int16_t low_value_vector[kNumChannels * 16];
    // TODO(bjornv): Change to |median|.
    int16_t mean_value[kNumChannels];
    int16_t upper_state[5];
    int16_t lower_state[5];
    int16_t hp_filter_state[4];
    int16_t over_hang_max_1[3];
    int16_t over_hang_max_2[3];
    int16_t individual[3];
    int16_t total[3];
    int init_flag;
} VadInstT;

// Initializes the core VAD component. The default aggressiveness mode is 0.
int WebRtcVad_InitCore(VadInstT* self);

// Sets the aggressiveness mode of a VAD instance. Valid modes are 0 to 3,
// where 0 is the least aggressive (sensitive) and 3 the most aggressive.
// The return value is 0 on success, or -1 if |mode| is invalid.
int WebRtcVad_set_mode_core(VadInstT* self, int mode);

// Runs the core VAD. One call processes 80 samples (10 ms at 8 kHz).
// Returns 1 if the audio is active, 0 if it is not, and -1 on error.
int WebRtcVad_CalcVad(int sample_rate, const int16_t* speech_frame,
                      int frame_length, VadInstT* inst);

#endif  // VAD_VAD_CORE_H_
