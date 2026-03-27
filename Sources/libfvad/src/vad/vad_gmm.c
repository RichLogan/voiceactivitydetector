/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vad_gmm.h"

#include <stdint.h>

#include "signal_processing/signal_processing_library.h"

static const int32_t kCompVar = 22005;
static const int16_t kLog2Exp = 5909;  // log2(exp(1)) in Q12.

// For a Gaussian probability calculation. For details see the comment in
// vad_gmm.h
int32_t WebRtcVad_GaussianProbability(int16_t input,
                                      int16_t mean,
                                      int16_t std,
                                      int16_t* delta) {
  int16_t tmp16, inv_std, inv_std2, exp_value;
  int32_t tmp32, inv_std_factor;

  // Calculate |inv_std| = 1 / |std| in Q10
  // Note that |std| > 0. Division: (2^17) / std, result in Q10.
  tmp32 = WebRtcSpl_DivW32W16((int32_t)131072, std);
  inv_std = (int16_t) WebRtcSpl_SatW32ToW16(tmp32);

  // |inv_std2| = |inv_std|^2 / 2 in Q10
  inv_std2 = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(inv_std, inv_std, 10);

  tmp16 = input - mean;  // Q4

  // |delta| = input for the update procedure in Q11
  *delta = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(tmp16, inv_std2, 3);

  // Calculate the exponent:
  // tmp16 = (|input| - |mean|)^2 / (2 * |std|^2) in Q4
  tmp32 = WEBRTC_SPL_MUL_16_16(tmp16, tmp16);  // Q8
  tmp32 = WEBRTC_SPL_MUL_16_16_RSFT(inv_std2, (int16_t) (tmp32 >> 2), 5);

  // |exp_value| = exp(|tmp32| * (-1)) in Q10
  exp_value = 0;
  if (tmp32 < kCompVar) {
    // Approximation of 2^(-|tmp32| * log2(e)) in Q10, where
    // log2(e) = kLog2Exp
    tmp32 = (int32_t) kLog2Exp * (-tmp32);  // Q12 (kLog2Exp is Q12)
    tmp32 += 32768;  // Rounding
    tmp32 >>= 16;  // Q(-4)?
    exp_value = (int16_t)(0x0400 >> (-tmp32));
    if (exp_value == 0) exp_value = 1;
  }

  // |tmp16| = |inv_std| * |exp_value| in Q20, but with |inv_std| in Q10
  // and |exp_value| in Q10 we get Q20 / 2^? depending on the shift.
  inv_std_factor = WEBRTC_SPL_MUL_16_16(inv_std, exp_value);

  return inv_std_factor;
}
