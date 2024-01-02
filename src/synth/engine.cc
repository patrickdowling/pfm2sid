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

#include <cmath>

#include "misc/platform.h"
#include "pfm2sid_stats.h"
#include "sidbits/sidbits.h"
#include "stm32x/stm32x_core.h"
#include "stm32x/stm32x_debug.h"
#include "synth.h"

ENABLE_WCONVERSION();

namespace pfm2sid::synth {

void Engine::Init(Parameters *parameters)
{
  parameters_ = parameters;

  sid_.set_sampling_parameters(sidbits::CLOCK_FREQ_PAL, reSID::SAMPLE_FAST, kDacUpdateRateHz);
  set_chip_model();
}

void Engine::Reset()
{
  cached_registers_.Reset();
  sid_.reset();
}

void Engine::SystemParameterChanged(SYSTEM parameter)
{
  (void)parameter;
}

void Engine::GlobalParameterChanged(GLOBAL parameter)
{
  if (GLOBAL::CHIP_MODEL == parameter) { set_chip_model(); }
}

void Engine::set_chip_model()
{
  if (parameters_->get<GLOBAL::CHIP_MODEL>().value()) {
    sid_.set_chip_model(reSID::MOS8580);
  } else {
    sid_.set_chip_model(reSID::MOS6581);
  }
  Reset();
}

static short render_buffer[kSampleBlockSize] INCCMZ;

void Engine::RenderBlock(SampleBuffer::MutableBlock block, const sidbits::RegisterMap &register_map)
{
  WriteRegisterMap(register_map);

  {
    stm32x::ScopedCycleMeasurement scm{stats::sid_clock_cycles};
    auto delta_t = clock_delta_t;
    sid_.clock(delta_t, render_buffer, kSampleBlockSize, 1);
  }

  auto src = render_buffer;
  for (auto &dst : block) { dst.left = dst.right = (*src++) << 2; }
}

}  // namespace pfm2sid::synth