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
#include "glide.h"

#include <cmath>

#include "synth.h"
#include "util/util_lut.h"

namespace pfm2sid::synth {

static constexpr float exponent = .125;
static constexpr float range = static_cast<float>(Glide::value_type::kFracMult);

static LUT_GENERATOR_CONSTEXPR float min_rate =
    powf(range / (kGlideTimeMin * kModulatorUpdateRateHz), -exponent);
static LUT_GENERATOR_CONSTEXPR float max_rate =
    powf(range / (kGlideTimeMax * kModulatorUpdateRateHz), -exponent);

LUT_GENERATOR_CONSTEXPR auto glide_increment(size_t i, size_t N)
{
  auto n = min_rate + (max_rate - min_rate) * static_cast<float>(i + 1) / static_cast<float>(N);
  return static_cast<int32_t>(powf(n, -1.f / exponent));
}

LUT_CONSTEXPR auto GLIDE_INCREMENT_TABLE =
    util::LookupTable<int32_t, 128>::generate(glide_increment);

int32_t Glide::glide_increment(int32_t rate)
{
  return GLIDE_INCREMENT_TABLE[rate];
}

}  // namespace pfm2sid::synth
