/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VAD_VAD_SP_H_
#define VAD_VAD_SP_H_

#include "vad_core.h"

// Downsamples the signal by a factor 2, 4 or 6 depending on |fs|.
//
// Input:
//      - inst          : VAD instance, which stores the filter states.
//      - signal        : Input signal.
//      - signal_length : Length of input signal.
//      - fs            : Sample rate of input signal, 32000 or 48000 Hz.
//
// Output:
//      - out_signal    : Downsampled signal.
//
// Return:
//      : Length of the output signal.
int WebRtcVad_Downsampling(const int16_t* signal, int16_t* out_signal,
                            int32_t* filter_state, int signal_length);

// Updates and returns the smoothed noise energy level in the |frequency_bands|
// frequency bands. Before the smoothing, a median replacement is also performed.
//
// Input:
//      - inst              : VAD instance which stores the history buffer.
//      - feature_value     : Feature value of current frame.
//      - channel_number    : Channel number.
//
// Return:
//      : Smoothed noise energy level in current channel.
int16_t WebRtcVad_FindMinimum(VadInstT* inst, int16_t feature_value,
                               int channel_number);

#endif  // VAD_VAD_SP_H_
