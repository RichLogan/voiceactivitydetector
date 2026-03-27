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
 * This file contains the downsampling by two function.
 * The description header can be found in signal_processing_library.h
 *
 */

#include <stdint.h>

#include "signal_processing_library.h"
#include "resample_by_2_internal.h"

void WebRtcSpl_DownsampleBy2(const int16_t *in, size_t len,
                              int16_t *out, int32_t *filtState)
{
    int32_t tmp[512];

    WebRtcSpl_DownBy2ShortToInt(in, (int32_t)len, tmp, filtState);
    WebRtcSpl_DownBy2IntToShort(tmp, (int32_t)(len / 2), out, filtState + 12);
}
