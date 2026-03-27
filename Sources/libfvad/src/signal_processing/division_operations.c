/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "signal_processing_library.h"

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den)
{
    // Guard against division with 0
    if (den != 0) {
        return (int32_t)(num / den);
    } else {
        return (int32_t)0x7FFFFFFF;
    }
}

int16_t WebRtcSpl_DivW32W16ResW16(int32_t num, int16_t den)
{
    // Guard against division with 0
    if (den != 0) {
        return (int16_t)(num / den);
    } else {
        return (int16_t)0x7FFF;
    }
}

int32_t WebRtcSpl_DivResultInQ31(int32_t num, int32_t den)
{
    int32_t L_num = num;
    int32_t L_den = den;
    int32_t div = 0;
    int k = 31;
    int change_sign = 0;

    if (num == 0)
        return 0;

    if (num < 0) {
        change_sign++;
        L_num = -num;
    }
    if (den < 0) {
        change_sign++;
        L_den = -den;
    }
    do {
        div <<= 1;
        L_num <<= 1;
        if (L_num >= L_den) {
            L_num -= L_den;
            div++;
        }
    } while (--k);

    if (change_sign == 1) {
        div = -div;
    }
    return div;
}

int32_t WebRtcSpl_DivW32HiLow(int32_t num, int16_t hi, int16_t low)
{
    int16_t t16;
    int32_t t32, denom;

    t16 = WebRtcSpl_DivW32W16ResW16(num, hi);

    // t16 = num / hi
    // Calculate remainder
    t32 = WEBRTC_SPL_MUL_16_16(hi, t16);
    t32 = num - t32;

    // re-calculate num from remainder
    denom = WEBRTC_SPL_MUL_16_16(low, t16);
    t16 = (int16_t) WEBRTC_SPL_RSHIFT_W32(denom, 15);
    return (int32_t)(t16 + t16);
}
