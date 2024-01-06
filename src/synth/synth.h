// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2023-2024 Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef PFM2SID_SYNTH_H_
#define PFM2SID_SYNTH_H_

#include <cinttypes>

#include "sample_buffer.h"

namespace pfm2sid::synth {

static constexpr uint32_t kSampleBlockSize = 32;
static constexpr uint32_t kNumSampleBlocks = 4UL;
static constexpr uint32_t kDacUpdateRateHz = 44100;

static constexpr float kModulatorUpdateRateHz =
    static_cast<float>(kDacUpdateRateHz) / static_cast<float>(kSampleBlockSize);

static constexpr float kLfoFreqMax = 40.f;
// static constexpr float kLfoFreqMin =

static constexpr float kGlideTimeMax = 2.f;
static constexpr float kGlideTimeMin = 3.f / kModulatorUpdateRateHz;

struct Sample {
  int32_t left;
  int32_t right;
};

using SampleBuffer = SampleBufferT<Sample, kSampleBlockSize, kNumSampleBlocks>;

enum LFO_INDEX : unsigned { LFO1, LFO2, LFO3 };
static constexpr unsigned kNumLfos = 3;

enum struct VOICE_MODE { POLY, UNISON };

}  // namespace pfm2sid::synth

ENABLE_ENUM_TO_INDEX(pfm2sid::synth::LFO_INDEX);

#endif  // PFM2SID_SYNTH_H_
