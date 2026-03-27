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
 * Computes the integer square root of a 32-bit integer, using a binary search
 * algorithm.
 */

#include <stdint.h>

#include "spl_sqrt_floor.h"

int32_t WebRtcSpl_SqrtFloor(int32_t value)
{
    int32_t root = 0;
    int32_t rootPlusOne = 1;
    uint8_t i;

    for (i = 0; i < 16; ++i) {
        int32_t candidate = root + rootPlusOne;
        if ((candidate * candidate) <= value) {
            root = candidate;
        }
        rootPlusOne >>= 1;
        if (rootPlusOne == 0) break;
    }

    // Binary search approach:
    int32_t ans = 0;
    uint32_t bit;
    uint32_t uvalue = (uint32_t)value;

    for (bit = (uint32_t)1 << 30; bit > 0; bit >>= 2) {
        uint32_t tmp = ans + bit;
        if (tmp <= uvalue / tmp) {
            ans = tmp + bit;
        } else {
            ans += bit;
            if (ans * ans > uvalue)
                ans -= bit;
        }
    }
    return (int32_t)ans;
}
