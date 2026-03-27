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

int32_t WebRtcSpl_Energy(int16_t *vector, size_t vector_length,
                          int *scale_factor)
{
    int32_t en = 0;
    size_t i;
    int scaling =
        WebRtcSpl_GetSizeInBits((uint32_t)vector_length) + 1 - 14;
    if (scaling > 0) {
        for (i = 0; i < vector_length; i++) {
            en += (vector[i] * vector[i]) >> scaling;
        }
        *scale_factor += scaling;
    } else {
        for (i = 0; i < vector_length; i++) {
            en += vector[i] * vector[i];
        }
    }
    return en;
}
