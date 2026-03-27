/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * This file contains the resampling by two functions.
 * The description header can be found in signal_processing_library.h
 *
 */

#include <stdint.h>

#include "resample_by_2_internal.h"
#include "signal_processing_library.h"

// allpass filter coefficients.
static const int16_t kResampleAllpass[2][3] = {
    {821, 6110, 12500},
    {3050, 9368, 15063}
};

//
//   decimator
// input:  int32_t (shifted 15 positions to the left, + offset 16384) OVERWRITTEN!
// output: int16_t (saturated) (of length len/2)
// state:  filter state array; length = 12

void WebRtcSpl_DownBy2IntToShort(int32_t *in, int32_t len, int16_t *out,
                                  int32_t *filtState)
{
    int32_t tmp1, tmp2, diff, in32, out32;
    int32_t i;

    len >>= 1;

    /* upper allpass filter: odd input samples */
    for (i = len; i > 0; i--) {
        in32 = *in++;

        diff = in32 - filtState[1];
        tmp1 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[0][0], diff) + filtState[0];
        filtState[0] = in32;
        filtState[1] = tmp1;

        diff = tmp1 - filtState[3];
        tmp2 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[0][1], diff) + filtState[2];
        filtState[2] = tmp1;
        filtState[3] = tmp2;

        diff = tmp2 - filtState[5];
        filtState[4] = tmp2;
        filtState[5] = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[0][2], diff) + filtState[4];

        /* lower allpass filter: even input samples */
        in32 = *in++;

        diff = in32 - filtState[7];
        tmp1 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[1][0], diff) + filtState[6];
        filtState[6] = in32;
        filtState[7] = tmp1;

        diff = tmp1 - filtState[9];
        tmp2 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[1][1], diff) + filtState[8];
        filtState[8] = tmp1;
        filtState[9] = tmp2;

        diff = tmp2 - filtState[11];
        filtState[10] = tmp2;
        filtState[11] = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[1][2], diff) + filtState[10];

        /* add and downsample */
        out32 = (filtState[5] >> 1) + (filtState[11] >> 1);
        if (out32 > (int32_t)0x00007FFF)
            *out++ = 32767;
        else if (out32 < (int32_t)0xFFFF8000)
            *out++ = -32768;
        else
            *out++ = (int16_t)out32;
    }
}

//
//   decimator
// input:  int16_t
// output: int32_t (shifted 15 positions to the left, + offset 16384)
// state:  filter state array; length = 12

void WebRtcSpl_DownBy2ShortToInt(const int16_t *in, int32_t len, int32_t *out,
                                  int32_t *filtState)
{
    int32_t tmp1, tmp2, diff, in32, out32;
    int32_t i;

    len >>= 1;

    /* upper allpass filter: odd input samples */
    for (i = len; i > 0; i--) {
        in32 = (int32_t)(*in++) << 15;

        diff = in32 - filtState[1];
        tmp1 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[0][0], diff) + filtState[0];
        filtState[0] = in32;
        filtState[1] = tmp1;

        diff = tmp1 - filtState[3];
        tmp2 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[0][1], diff) + filtState[2];
        filtState[2] = tmp1;
        filtState[3] = tmp2;

        diff = tmp2 - filtState[5];
        filtState[4] = tmp2;
        filtState[5] = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[0][2], diff) + filtState[4];

        /* lower allpass filter: even input samples */
        in32 = (int32_t)(*in++) << 15;

        diff = in32 - filtState[7];
        tmp1 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[1][0], diff) + filtState[6];
        filtState[6] = in32;
        filtState[7] = tmp1;

        diff = tmp1 - filtState[9];
        tmp2 = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[1][1], diff) + filtState[8];
        filtState[8] = tmp1;
        filtState[9] = tmp2;

        diff = tmp2 - filtState[11];
        filtState[10] = tmp2;
        filtState[11] = WEBRTC_SPL_MUL_16_32_RSFT15(kResampleAllpass[1][2], diff) + filtState[10];

        /* add and downsample */
        out32 = (filtState[5] >> 1) + (filtState[11] >> 1);
        *out++ = out32;
    }
}
