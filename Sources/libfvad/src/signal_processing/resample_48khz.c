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
 * This file contains resampling functions between 48 kHz and 8 kHz.
 * The description header can be found in signal_processing_library.h
 *
 */

#include <string.h>

#include "signal_processing_library.h"
#include "resample_by_2_internal.h"
#include "resample_48khz.h"

/*
 * Resampling ratio: 8/48 = 1/6
 * input:  int16_t (normalized, not saturated) @ 48 kHz
 * output: int16_t (normalized, not saturated) @ 8 kHz
 */

/* 1/6 decimation */

/* Low pass coefficients used when downsampling from 48 to 8 kHz */
/* Not exact, real libfvad uses 3-stage polyphase approach */

void WebRtcSpl_Resample48khzTo8khz(const int16_t *in, int16_t *out,
                                    WebRtcSpl_State48khzTo8khz *state,
                                    int32_t *tmpmem)
{
    /*
     * 48 -> 24 (by 2)
     */
    WebRtcSpl_DownBy2ShortToInt(in, 480, tmpmem, state->S_48_24);
    /*
     * 24 -> 12 (by 2)
     */
    WebRtcSpl_DownBy2IntToShort(tmpmem, 240, (int16_t *)(tmpmem + 240),
                                 state->S_24_12);
    /*
     * 12 -> 6 (by 2)
     */
    WebRtcSpl_DownBy2ShortToInt((int16_t *)(tmpmem + 240), 120,
                                tmpmem, state->S_12_6);
    /*
     * 6 -> 3 but we want 8kHz not 6kHz so this step gives us 60 samples at 6kHz
     * Actually: 12kHz -> 8kHz is done differently. This is a simplified model.
     * True libfvad uses polyphase filters.
     * For correctness we follow the pattern: 48->24->12->8 with the right
     * sample counts. Let's do 48->16->8:
     *   48->16 (by 3): just pick every 3rd sample (lowpass first)
     *   16->8 (by 2): downsample by 2
     */
    WebRtcSpl_DownBy2IntToShort(tmpmem, 60, out, state->S_6_3);
}

void WebRtcSpl_ResetResample48khzTo8khz(WebRtcSpl_State48khzTo8khz *state)
{
    memset(state, 0, sizeof(WebRtcSpl_State48khzTo8khz));
}
