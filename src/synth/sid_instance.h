// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2024 Patrick Dowling (pld@gurkenkiste.com)
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
#ifndef PFM2SID_SID_INSTANCE_H_
#define PFM2SID_SID_INSTANCE_H_

#include <cmath>

#include "sid.h"
#include "sidbits/sidbits.h"
#include "synth/synth.h"

namespace pfm2sid::synth {

class SIDInstance {
public:
  // \sa RenderBlock
  static constexpr reSID::cycle_count clock_delta_t =
      ceilf(sidbits::CLOCK_FREQ_PAL / (float)kDacUpdateRateHz * (float)kSampleBlockSize);

  void Init(reSID::chip_model chip_model);
  void Reset();

  const auto &register_map() const { return cached_registers_; }
  void set_chip_model(reSID::chip_model chip_model);

  inline void Render(reSID::output_sample_t *dst, int n, const sidbits::RegisterMap &register_map)
  {
    WriteRegisterMap(register_map);
    auto delta_t = clock_delta_t;
    sid_.clock(delta_t, dst, n, 1);
  }

private:
  sidbits::RegisterMap cached_registers_;
  reSID::SID sid_;

  void WriteRegisterMap(const sidbits::RegisterMap &register_map)
  {
    for (reSID::reg8 r = 0; r < register_map.kNumRegisters; ++r)
      WriteRegister(r, register_map.peek(r));
  }

  inline void WriteRegister(reSID::reg8 r, uint8_t value)
  {
    if (cached_registers_.peek(r) != value) {
      sid_.write(r, value);
      cached_registers_.poke(r, value);
    }
  }
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SID_INSTANCE_H_
