/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VAD_VAD_FILTERBANK_H_
#define VAD_VAD_FILTERBANK_H_

#include "vad_core.h"

// Calculates the energy for each of the |kNumChannels| = 6 frequency bands
// used by the VAD:
//    80 Hz - 250 Hz
//    250 Hz - 500 Hz
//    500 Hz - 1000 Hz
//    1000 Hz - 2000 Hz
//    2000 Hz - 3000 Hz
//    3000 Hz - 4000 Hz
//
// The signals are filtered out using half-band filters recursively, after
// which the energy in each frequency band of interest is calculated.
//
// - inst           [i/o] : State information of the VAD.
// - data_in        [i]   : Input audio data.
// - data_length    [i]   : Length of input audio data.
// - features       [o]   : 10 * log10(energy) of each of the 6 bands.
//
// - returns              : Total power of the signal (over all channels) in
//                          linear scale
int16_t WebRtcVad_LogOfEnergy(VadInstT* inst, const int16_t* data_in,
                               size_t data_length, int16_t offset,
                               int16_t* features);

#endif  // VAD_VAD_FILTERBANK_H_
