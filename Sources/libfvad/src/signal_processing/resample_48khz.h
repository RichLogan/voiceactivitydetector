/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SIGNAL_PROCESSING_RESAMPLE_48KHZ_H_
#define SIGNAL_PROCESSING_RESAMPLE_48KHZ_H_

#include <stdint.h>

typedef struct {
    int32_t S_48_24[8];
    int32_t S_24_12[8];
    int32_t S_12_6[8];
    int32_t S_6_3[8];
} WebRtcSpl_State48khzTo8khz;

void WebRtcSpl_Resample48khzTo8khz(const int16_t *in, int16_t *out,
                                    WebRtcSpl_State48khzTo8khz *state,
                                    int32_t *tmpmem);

void WebRtcSpl_ResetResample48khzTo8khz(WebRtcSpl_State48khzTo8khz *state);

typedef struct {
    int32_t S_8_16[8];
    int32_t S_16_32[8];
    int32_t S_32_24[8];
    int32_t S_24_48[8];
} WebRtcSpl_State8khzTo48khz;

void WebRtcSpl_Resample8khzTo48khz(const int16_t *in, int16_t *out,
                                    WebRtcSpl_State8khzTo48khz *state,
                                    int32_t *tmpmem);

void WebRtcSpl_ResetResample8khzTo48khz(WebRtcSpl_State8khzTo48khz *state);

#endif  // SIGNAL_PROCESSING_RESAMPLE_48KHZ_H_
