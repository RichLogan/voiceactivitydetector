/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "fvad.h"

#include <stdlib.h>
#include <string.h>

#include "vad/vad_core.h"
#include "signal_processing/signal_processing_library.h"

struct Fvad {
    VadInstT core;
    int sample_rate;
    int rate_index;
};

Fvad *fvad_new(void)
{
    Fvad *inst = (Fvad *) calloc(1, sizeof *inst);
    if (inst) {
        fvad_reset(inst);
    }
    return inst;
}

void fvad_free(Fvad *inst)
{
    free(inst);
}

void fvad_reset(Fvad *inst)
{
    WebRtcVad_InitCore(&inst->core);
    inst->sample_rate = 8000;
    inst->rate_index  = 0;
}

int fvad_set_mode(Fvad *inst, int mode)
{
    return WebRtcVad_set_mode_core(&inst->core, mode);
}

int fvad_set_sample_rate(Fvad *inst, int sample_rate)
{
    int rate_index;
    switch (sample_rate) {
        case 8000:  rate_index = 0; break;
        case 16000: rate_index = 1; break;
        case 32000: rate_index = 2; break;
        case 48000: rate_index = 3; break;
        default: return -1;
    }
    inst->sample_rate = sample_rate;
    inst->rate_index  = rate_index;
    return 0;
}

int fvad_process(Fvad *inst, const int16_t *frame, size_t length)
{
    /* Permissible frame lengths per sample rate (10, 20, 30 ms) */
    static const size_t valid_lengths[4][3] = {
        {  80,  160,  240 },   /* 8000 Hz */
        { 160,  320,  480 },   /* 16000 Hz */
        { 320,  640,  960 },   /* 32000 Hz */
        { 480,  960, 1440 },   /* 48000 Hz */
    };

    const size_t *vl = valid_lengths[inst->rate_index];
    if (length != vl[0] && length != vl[1] && length != vl[2]) {
        return -1;
    }

    return WebRtcVad_CalcVad(inst->sample_rate, frame, (int)length,
                             &inst->core);
}
