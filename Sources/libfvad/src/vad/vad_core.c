/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vad_core.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "signal_processing_library.h"
#include "resample_48khz.h"
#include "resample_by_2.h"
#include "vad_filterbank.h"
#include "vad_gmm.h"
#include "vad_sp.h"

// Spectrum width of each sub-band in 8 kHz domain [Hz].
// 80 Hz - 250 Hz
// 250 Hz - 500 Hz
// 500 Hz - 1000 Hz
// 1000 Hz - 2000 Hz
// 2000 Hz - 3000 Hz
// 3000 Hz - 4000 Hz
static const int16_t kSpectrumWeight[kNumChannels] = { 6, 8, 10, 12, 14, 16 };
static const int16_t kNoiseUpdateConst = 655;   // Q15
static const int16_t kSpeechUpdateConst = 6554; // Q15
static const int16_t kBackEta = 154;            // Q8
// Minimum speech/noise weights.
static const int16_t kMinimumMean[kNumGaussians] = { 10, 36 };
// Over-hang values for the three aggressiveness modes. All modes use 0 frames
// as over-hang when not triggered, otherwise they use:
// Mode 0: maximum over-hang  8 frames
// Mode 1: maximum over-hang  4 frames
// Mode 2: maximum over-hang  2 frames
// Mode 3: maximum over-hang  1 frame
static const int16_t kOverHangMax1[3] = { 8, 4, 3 };
static const int16_t kOverHangMax2[3] = { 14, 7, 5 };

// Thresholds for the three different individual modes (0, 1, 2, 3)
// Mode 0  - the 3 individual thresholds are lowered compared to the other modes
// Mode 1
// Mode 2
// Mode 3  - the 3 individual thresholds are 3 times as large as mode 0
static const int16_t kLocalThreshold[3] = { 24, 21, 24 };
static const int16_t kGlobalThreshold[3] = { 57, 48, 57 };
static const int16_t kIndividual[3] = { 24, 21, 24 };
static const int16_t kTotal[3] = { 57, 48, 57 };

// Initial values for the Gaussian probability model.
// Noise:
static const int16_t kNoiseDataMeans[kTableSize] = {
    6732, 8689, 9444, 10241, 10846, 11601,
    6732, 8689, 9444, 10241, 10846, 11601
};
static const int16_t kNoiseDataStds[kTableSize] = {
    378, 1064, 493, 582, 688, 756,
    378, 1064, 493, 582, 688, 756
};
// Speech:
static const int16_t kSpeechDataMeans[kTableSize] = {
    8306, 9560, 10286, 11048, 11624, 12204,
    8306, 9560, 10286, 11048, 11624, 12204
};
static const int16_t kSpeechDataStds[kTableSize] = {
    378, 1064, 493, 582, 688, 756,
    378, 1064, 493, 582, 688, 756
};

// Initialize GMM parameters.
static void InitializeGMM(VadInstT* self) {
    memcpy(self->noise_means, kNoiseDataMeans, sizeof(self->noise_means));
    memcpy(self->noise_stds, kNoiseDataStds, sizeof(self->noise_stds));
    memcpy(self->speech_means, kSpeechDataMeans, sizeof(self->speech_means));
    memcpy(self->speech_stds, kSpeechDataStds, sizeof(self->speech_stds));
}

int WebRtcVad_InitCore(VadInstT* self) {
    int i;

    if (self == NULL) {
        return -1;
    }

    memset(self, 0, sizeof(*self));

    // Initialize downsampling filter states.
    memset(self->downsampling_filter_states, 0,
           sizeof(self->downsampling_filter_states));

    // Initialize 48 kHz to 8 kHz resampler state.
    WebRtcSpl_ResetResample48khzTo8khz(&self->state_48_to_8);

    // Initialize GMM.
    InitializeGMM(self);

    // Initialize over-hang values.
    self->over_hang_max_1[0] = kOverHangMax1[0];
    self->over_hang_max_1[1] = kOverHangMax1[1];
    self->over_hang_max_1[2] = kOverHangMax1[2];

    self->over_hang_max_2[0] = kOverHangMax2[0];
    self->over_hang_max_2[1] = kOverHangMax2[1];
    self->over_hang_max_2[2] = kOverHangMax2[2];

    self->individual[0] = kIndividual[0];
    self->individual[1] = kIndividual[1];
    self->individual[2] = kIndividual[2];

    self->total[0] = kTotal[0];
    self->total[1] = kTotal[1];
    self->total[2] = kTotal[2];

    self->init_flag = 42;
    self->frame_counter = 0;
    self->over_hang = 0;
    self->num_of_speech = 0;

    // Initialize |mean_value|.
    for (i = 0; i < kNumChannels; i++) {
        self->mean_value[i] = 1600;
    }

    return 0;
}

int WebRtcVad_set_mode_core(VadInstT* self, int mode) {
    int return_value = 0;

    switch (mode) {
        case 0:
            // Quality mode.
            memcpy(self->over_hang_max_1, kOverHangMax1,
                   sizeof(self->over_hang_max_1));
            memcpy(self->over_hang_max_2, kOverHangMax2,
                   sizeof(self->over_hang_max_2));
            self->individual[0] = kIndividual[0];
            self->individual[1] = kIndividual[1];
            self->individual[2] = kIndividual[2];
            self->total[0] = kTotal[0];
            self->total[1] = kTotal[1];
            self->total[2] = kTotal[2];
            break;
        case 1:
            self->over_hang_max_1[0] = 6;
            self->over_hang_max_1[1] = 4;
            self->over_hang_max_1[2] = 3;
            self->over_hang_max_2[0] = 9;
            self->over_hang_max_2[1] = 7;
            self->over_hang_max_2[2] = 5;
            self->individual[0] = 24;
            self->individual[1] = 21;
            self->individual[2] = 24;
            self->total[0] = 57;
            self->total[1] = 48;
            self->total[2] = 57;
            break;
        case 2:
            self->over_hang_max_1[0] = 3;
            self->over_hang_max_1[1] = 2;
            self->over_hang_max_1[2] = 2;
            self->over_hang_max_2[0] = 5;
            self->over_hang_max_2[1] = 4;
            self->over_hang_max_2[2] = 3;
            self->individual[0] = 36;
            self->individual[1] = 32;
            self->individual[2] = 36;
            self->total[0] = 100;
            self->total[1] = 80;
            self->total[2] = 100;
            break;
        case 3:
            self->over_hang_max_1[0] = 2;
            self->over_hang_max_1[1] = 1;
            self->over_hang_max_1[2] = 1;
            self->over_hang_max_2[0] = 3;
            self->over_hang_max_2[1] = 2;
            self->over_hang_max_2[2] = 2;
            self->individual[0] = 48;
            self->individual[1] = 44;
            self->individual[2] = 48;
            self->total[0] = 116;
            self->total[1] = 96;
            self->total[2] = 116;
            break;
        default:
            return_value = -1;
            break;
    }

    return return_value;
}

// Calculate VAD decision and update models for a 10ms 8 kHz frame.
static int VadVoiceActivity(VadInstT* inst,
                             const int16_t* features,
                             int16_t total_power,
                             int mode_index) {
    int16_t noise_prob[kNumChannels], speech_prob[kNumChannels];
    int16_t vadflag = 0;
    int16_t noise_global_mean, speech_global_mean;
    int32_t noise_sum = 0, speech_sum = 0;
    int h, i, k;
    int16_t speech_delta, noise_delta;
    int16_t noise_probability, speech_probability;

    if (total_power <= kMinEnergy) {
        return 0;  // Too quiet, return inactive.
    }

    for (i = 0; i < kNumChannels; i++) {
        noise_prob[i] = 0;
        speech_prob[i] = 0;
        for (h = 0; h < kNumGaussians; h++) {
            k = i + h * kNumChannels;
            noise_probability = WebRtcVad_GaussianProbability(
                features[i], inst->noise_means[k], inst->noise_stds[k],
                &noise_delta);
            speech_probability = WebRtcVad_GaussianProbability(
                features[i], inst->speech_means[k], inst->speech_stds[k],
                &speech_delta);
            noise_prob[i] += (int16_t)(noise_probability >> 10);
            speech_prob[i] += (int16_t)(speech_probability >> 10);
        }
    }

    for (i = 0; i < kNumChannels; i++) {
        noise_sum += noise_prob[i] * kSpectrumWeight[i];
        speech_sum += speech_prob[i] * kSpectrumWeight[i];
    }

    noise_global_mean = (int16_t)(noise_sum >> 9);
    speech_global_mean = (int16_t)(speech_sum >> 9);

    if (speech_global_mean > noise_global_mean) {
        vadflag = 1;
    }

    // Update over-hang counter.
    if (vadflag) {
        inst->over_hang = inst->over_hang_max_2[mode_index];
        inst->num_of_speech++;
    } else {
        if (inst->over_hang > 0) {
            vadflag = 1;
            inst->over_hang--;
        }
        inst->num_of_speech = 0;
    }

    return vadflag;
}

int WebRtcVad_CalcVad(int sample_rate,
                      const int16_t* speech_frame,
                      int frame_length,
                      VadInstT* inst) {
    int16_t feature_vector[kNumChannels];
    int16_t total_power = 0;
    int16_t speech_nb[480];  // Temporary for downsampled frames
    int32_t resamp_tmp[512];
    int16_t speech_16khz[240];
    int16_t speech_8khz[120];
    int mode_index;
    (void)speech_16khz;
    (void)speech_8khz;

    if (inst->init_flag != 42) {
        return -1;
    }

    inst->frame_counter++;

    // Mode index for over-hang thresholds.
    mode_index = 0;

    // Resample to 8 kHz based on sample_rate.
    if (sample_rate == 48000) {
        WebRtcSpl_Resample48khzTo8khz(speech_frame, speech_nb,
                                       &inst->state_48_to_8, resamp_tmp);
        frame_length = 80;
    } else if (sample_rate == 32000) {
        WebRtcSpl_DownsampleBy2(speech_frame, (size_t)frame_length,
                                 speech_nb, inst->downsampling_filter_states);
        frame_length /= 2;
        WebRtcSpl_DownsampleBy2(speech_nb, (size_t)frame_length,
                                 speech_nb, inst->downsampling_filter_states + 2);
        frame_length /= 2;
    } else if (sample_rate == 16000) {
        WebRtcSpl_DownsampleBy2(speech_frame, (size_t)frame_length,
                                 speech_nb, inst->downsampling_filter_states);
        frame_length /= 2;
    } else {
        memcpy(speech_nb, speech_frame,
               (size_t)frame_length * sizeof(int16_t));
    }

    // Calculate features (log energy per band).
    total_power = WebRtcVad_LogOfEnergy(inst, speech_nb, (size_t)frame_length,
                                         0, feature_vector);

    return VadVoiceActivity(inst, feature_vector, total_power, mode_index);
}
