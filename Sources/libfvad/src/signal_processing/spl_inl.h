/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Inline functions for WebRTC SPL

#ifndef SIGNAL_PROCESSING_SPL_INL_H_
#define SIGNAL_PROCESSING_SPL_INL_H_

#include <stdint.h>

static __inline int WebRtcSpl_NormW32Inline(int32_t a) {
    int zeros;
    if (a <= 0) a ^= 0xFFFFFFFF;
    if (!(0xFFFF8000 & a)) { zeros = 16; a <<= 16; } else { zeros = 0; }
    if (!(0xFF800000 & a)) { zeros += 8; a <<= 8;  }
    if (!(0xF8000000 & a)) { zeros += 4; a <<= 4;  }
    if (!(0xE0000000 & a)) { zeros += 2; a <<= 2;  }
    if (!(0xC0000000 & a)) { zeros += 1; }
    return zeros;
}

#endif  // SIGNAL_PROCESSING_SPL_INL_H_
