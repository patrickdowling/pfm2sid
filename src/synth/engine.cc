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
#include "engine.h"

#include "misc/platform.h"
#include "pfm2sid_stats.h"
#include "stm32x/stm32x_core.h"
#include "stm32x/stm32x_debug.h"
#include "synth.h"

ENABLE_WCONVERSION()

namespace pfm2sid::synth {

namespace detail {
template <>
constexpr auto typed_value<reSID::chip_model>(pfm2sid::synth::parameter_value_type value)
{
  return value ? reSID::MOS8580 : reSID::MOS6581;
}
}  // namespace detail

void Engine::Init(Parameters *parameters)
{
  parameters_ = parameters;
  sid_instance_.Init(parameters_->get<GLOBAL::CHIP_MODEL, reSID::chip_model>());
}

void Engine::Reset()
{
  sid_instance_.Reset();
}

void Engine::SystemParameterChanged(SYSTEM parameter)
{
  (void)parameter;
}

void Engine::GlobalParameterChanged(GLOBAL parameter)
{
  if (GLOBAL::CHIP_MODEL == parameter) {
    sid_instance_.set_chip_model(parameters_->get<GLOBAL::CHIP_MODEL, reSID::chip_model>());
  }
}

static reSID::output_sample_t render_buffer[kSampleBlockSize] INCCMZ;

// NOTE
// With an incorrect clock_delta_t value, the single call to clock(...) doesn't return
// kSampleBlockSize samples. It could either be called in a while loop until we have enough, but
// using ceil to calculate the factor also "seems to work". There's probably also a way to calculate
// the error and work around it that way (the sample tracking internally is a 16.16 value).

void Engine::RenderBlock(SampleBuffer::MutableBlock block, const sidbits::RegisterMap &register_map)
{
  {
    stm32x::ScopedCycleMeasurement scm{stats::sid_clock_cycles};
    sid_instance_.Render(render_buffer, kSampleBlockSize, register_map);
  }

  auto src = render_buffer;
  for (auto &dst : block) {
    auto s = *src++ >> 2;
    dst.left = dst.right = __SSAT(s, 18);
  }
}

}  // namespace pfm2sid::synth
