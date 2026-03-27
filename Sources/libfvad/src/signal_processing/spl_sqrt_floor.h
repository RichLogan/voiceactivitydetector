/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SIGNAL_PROCESSING_SPL_SQRT_FLOOR_H_
#define SIGNAL_PROCESSING_SPL_SQRT_FLOOR_H_

#include <stdint.h>

// Sqrt of integer x, rounded down to nearest integer, returning int32_t.
int32_t WebRtcSpl_SqrtFloor(int32_t value);

#endif  // SIGNAL_PROCESSING_SPL_SQRT_FLOOR_H_
