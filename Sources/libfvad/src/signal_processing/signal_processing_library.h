/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SIGNAL_PROCESSING_LIBRARY_H_
#define SIGNAL_PROCESSING_LIBRARY_H_

#include <stdint.h>
#include <string.h>

// Macros specific for the fixed point implementation
#define WEBRTC_SPL_WORD16_MAX       32767
#define WEBRTC_SPL_WORD16_MIN       -32768
#define WEBRTC_SPL_WORD32_MAX       (int32_t)0x7fffffff
#define WEBRTC_SPL_WORD32_MIN       (int32_t)0x80000000
#define WEBRTC_SPL_MAX_LPC_ORDER    14
#define WEBRTC_SPL_MIN(A, B)        (A < B ? A : B)
#define WEBRTC_SPL_MAX(A, B)        (A > B ? A : B)
#define WEBRTC_SPL_ABS_W16(a) \
    (((int16_t)a >= 0) ? ((int16_t)a) : -((int16_t)a))
#define WEBRTC_SPL_ABS_W32(a) \
    (((int32_t)a >= 0) ? ((int32_t)a) : -((int32_t)a))

#define WEBRTC_SPL_MUL(a, b) \
    ((int32_t) ((int32_t)(a) * (int32_t)(b)))
#define WEBRTC_SPL_UMUL(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint32_t)(b)))
#define WEBRTC_SPL_UMUL_16_16(a, b) \
    ((uint32_t) (uint16_t)(a) * (uint16_t)(b))
#define WEBRTC_SPL_UMUL_16_16_RSFT16(a, b) \
    (((uint32_t) (uint16_t)(a) * (uint16_t)(b)) >> 16)
#define WEBRTC_SPL_UMUL_32_16(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint16_t)(b)))
#define WEBRTC_SPL_UMUL_32_16_RSFT16(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint16_t)(b)) >> 16)
#define WEBRTC_SPL_MUL_16_U16(a, b) \
    ((int32_t)(int16_t)(a) * (uint16_t)(b))
#define WEBRTC_SPL_DIV(a, b) \
    ((int32_t) ((int32_t)(a) / (int32_t)(b)))
#define WEBRTC_SPL_UDIV(a, b) \
    ((uint32_t) ((uint32_t)(a) / (uint32_t)(b)))

#ifndef WEBRTC_ARCH_ARM_V7
// For ARMv7 platforms, these are defined in:
// common_audio/signal_processing/include/spl_inl_armv7.h
#define WEBRTC_SPL_MUL_16_16(a, b) \
    ((int32_t) (((int16_t)(a)) * ((int16_t)(b))))
#define WEBRTC_SPL_MUL_16_32_RSFT16(a, b) \
    (WEBRTC_SPL_MUL_16_16(a, b >> 16) \
     + ((WEBRTC_SPL_MUL_16_16(a, (b & 0xffff) >> 1) + 0x4000) >> 15))
#endif

#define WEBRTC_SPL_MUL_16_32_RSFT11(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 5) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x0200) >> 10))
#define WEBRTC_SPL_MUL_16_32_RSFT14(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 2) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x1000) >> 13))
#define WEBRTC_SPL_MUL_16_32_RSFT15(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 1) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x2000) >> 14))

#define WEBRTC_SPL_MUL_16_16_RSFT(a, b, c) \
    (WEBRTC_SPL_MUL_16_16(a, b) >> (c))
#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(a, b, c) \
    ((WEBRTC_SPL_MUL_16_16(a, b) + ((int32_t) \
                                  (((int32_t)1) << ((c) - 1)))) >> (c))
#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_FIXROUND(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, b) + ((int32_t) (1 << 14))) >> 15)

// C + the 32 most significant bits of A * B
#define WEBRTC_SPL_SCALEDIFF32(A, B, C) \
    (C + (B >> 16) * A + (((uint32_t)(0x0000FFFF & B) * A) >> 16))

#define WEBRTC_SPL_SAT(a, b, c)         (b > a ? a : b < c ? c : b)

// Shifting with negative numbers allowed
// Positive means left shift
#define WEBRTC_SPL_SHIFT_W32(x, c) \
    (((c) >= 0) ? ((x) << (c)) : ((x) >> (-(c))))

// Shifting with negative numbers not allowed
// We cannot do casting here due to signed/unsigned problem
#define WEBRTC_SPL_LSHIFT_W32(x, c)     ((x) << (c))
#define WEBRTC_SPL_RSHIFT_W32(x, c)     ((x) >> (c))
#define WEBRTC_SPL_LSHIFT_U32(x, c)     ((uint32_t)(x) << (c))

#define WEBRTC_SPL_RSFTW32_MUL_16(a, b, c) \
    (int32_t)(((int64_t)(a) * (b)) >> (c))
#define WEBRTC_SPL_MUL_32_32_RSFT32(a, b, c) \
    (int32_t)(((int64_t)(a) * (int64_t)(b)) >> (c))
#define WEBRTC_SPL_MUL_32_32_RSFT32BI(a, b) \
    (int32_t)(((int64_t)(a) * (int64_t)(b)) >> 32)

// Normalize functions
int16_t WebRtcSpl_GetSizeInBits(uint32_t n);
int WebRtcSpl_NormW32(int32_t a);
int WebRtcSpl_NormU32(uint32_t a);
int WebRtcSpl_NormW16(int16_t a);

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den);
int16_t WebRtcSpl_DivW32W16ResW16(int32_t num, int16_t den);
int32_t WebRtcSpl_DivResultInQ31(int32_t num, int32_t den);
int32_t WebRtcSpl_DivW32HiLow(int32_t num, int16_t hi, int16_t low);

int16_t WebRtcSpl_SatW32ToW16(int32_t value32);
int32_t WebRtcSpl_AddSatW32(int32_t a, int32_t b);
int16_t WebRtcSpl_AddSatW16(int16_t a, int16_t b);
int32_t WebRtcSpl_SubSatW32(int32_t var1, int32_t var2);
int16_t WebRtcSpl_SubSatW16(int16_t var1, int16_t var2);

int16_t WebRtcSpl_MaxAbsValueW16(const int16_t *vector, size_t length);
int32_t WebRtcSpl_MaxAbsValueW32(const int32_t *vector, size_t length);
int16_t WebRtcSpl_MaxValueW16(const int16_t *vector, size_t length);
int32_t WebRtcSpl_MaxValueW32(const int32_t *vector, size_t length);
int16_t WebRtcSpl_MinValueW16(const int16_t *vector, size_t length);
int32_t WebRtcSpl_MinValueW32(const int32_t *vector, size_t length);

void WebRtcSpl_DownsampleBy2(const int16_t *in, size_t len,
                              int16_t *out, int32_t *filtState);

#endif  /* SIGNAL_PROCESSING_LIBRARY_H_ */
