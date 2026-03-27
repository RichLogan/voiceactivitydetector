/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SIGNAL_PROCESSING_GET_SCALING_SQUARE_H_
#define SIGNAL_PROCESSING_GET_SCALING_SQUARE_H_

#include <stdint.h>

int16_t WebRtcSpl_GetScalingSquare(int16_t *in_vector,
                                    size_t in_vector_length,
                                    size_t times);

#endif  // SIGNAL_PROCESSING_GET_SCALING_SQUARE_H_
