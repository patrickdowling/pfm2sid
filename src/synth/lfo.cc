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
#include "lfo.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "misc/platform.h"
#include "synth.h"
#include "util/util_lut.h"

ENABLE_WCONVERSION()

namespace pfm2sid::synth {

void Lfo::Init(int32_t rate, int start_phase)
{
  phase_inc_ = phase_increment(rate);
  Reset(start_phase);
}

constexpr auto lfo_rate_to_freq(size_t i)
{
  return kLfoFreqMax * powf(2.f, -static_cast<float>(127 - i) / 12.f);
}

constexpr auto lfo_freq_to_phase(float f)
{
  return f / kModulatorUpdateRateHz;
}

constexpr auto phase_value(size_t i, size_t /*N*/)
{
  return lfo_freq_to_phase(lfo_rate_to_freq(i));
}

LUT_CONSTEXPR auto PHASE_INCREMENT_TABLE =
    util::LookupTable<float, 128, int32_t>::generate(phase_value);

/*static*/ float Lfo::phase_increment(int32_t rate)
{
  return PHASE_INCREMENT_TABLE.read(rate);
}

/*static*/ float Lfo::lfo_freq(int32_t rate)
{
  rate = std::clamp(rate, (int32_t)0, (int32_t)127);
  return lfo_rate_to_freq(static_cast<size_t>(rate));
}

constexpr auto sine_value(size_t i, size_t N)
{
  constexpr float pi = 3.14159265358979323846f;
  return sinf(2.f * pi * static_cast<float>(i) / static_cast<float>(N));
}

LUT_CONSTEXPR auto SINE_TABLE =
    util::LookupTable<float, Lfo::kSinTableSize, float, 1>::generate(sine_value);

/*static*/ float Lfo::sine_table(float phase)
{
  return SINE_TABLE.read_interpolated(phase);
}

}  // namespace pfm2sid::synth
