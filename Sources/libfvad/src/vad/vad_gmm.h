/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VAD_VAD_GMM_H_
#define VAD_VAD_GMM_H_

#include <stdint.h>

// Calculates the probability for |input|, given a Gaussian model, |mean| and
// |std|.
//
// Inputs:
//      - input         : input sample in Q4
//      - mean          : mean input in the statistical model, Q7
//      - std           : standard deviation, Q7
//
// Output:
//      - delta         : input to the learning process, Q11
//
// Return:
//      (probability in Q20) * 2^7
int32_t WebRtcVad_GaussianProbability(int16_t input,
                                      int16_t mean,
                                      int16_t std,
                                      int16_t *delta);

#endif  // VAD_VAD_GMM_H_
