/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SIGNAL_PROCESSING_RESAMPLE_BY_2_H_
#define SIGNAL_PROCESSING_RESAMPLE_BY_2_H_

#include <stdint.h>

void WebRtcSpl_DownsampleBy2(const int16_t *in, size_t len,
                              int16_t *out, int32_t *filtState);

#endif  // SIGNAL_PROCESSING_RESAMPLE_BY_2_H_
