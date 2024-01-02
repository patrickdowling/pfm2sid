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
#ifndef PFM2SID_SYNTH_MODULATION_H_
#define PFM2SID_SYNTH_MODULATION_H_

#include <algorithm>
#include <array>

#include "misc/fixed_point.h"
#include "util/util_templates.h"

namespace pfm2sid::synth {

enum struct MOD_SRC : unsigned { NONE, LFO1, LFO2, LFO3, PITCH_BEND, LAST };
}  // namespace pfm2sid::synth

ENABLE_ENUM_TO_INDEX(pfm2sid::synth::MOD_SRC);

namespace pfm2sid::synth {

static constexpr auto kNumModulationSrc = util::enum_count<MOD_SRC>();
static constexpr float kModulationDepth = 256.f;
static constexpr int32_t kModDepthMin = -256;
static constexpr int32_t kModDepthMax = 255;

using modulation_value_type = int32_t;

class ModulationValues {
public:
  void Reset() { std::fill(sources_.begin(), sources_.end(), 0.f); }

  modulation_value_type get(MOD_SRC mod_src, modulation_value_type depth, float range) const;

  template <MOD_SRC mod_src>
  float& mutable_src()
  {
    static_assert(MOD_SRC::NONE != mod_src);
    static_assert(MOD_SRC::LAST != mod_src);
    return sources_[util::enum_to_i(mod_src)];
  }

private:
  std::array<float, kNumModulationSrc> sources_;
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_MODULATION_H_
