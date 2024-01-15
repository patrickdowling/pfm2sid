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
#ifndef PFM2SID_ENGINE_H_
#define PFM2SID_ENGINE_H_

#include <cmath>

#include "sid.h"
#include "synth/parameter_listener.h"
#include "synth/parameter_structs.h"
#include "synth/synth.h"
#include "util/util_macros.h"

namespace pfm2sid::synth {

// Obligatory abstraction of sound generation. Engine? Processor? Generator?
//
// For now it's just a wrapper around a SID instance, and the actual "fun" happens in a wrapper
// (ASID player, synth, etc.) that can set a suitable register map...
//
class Engine : public ParameterListener {
public:
  // \sa RenderBlock
  static constexpr reSID::cycle_count clock_delta_t =
      ceilf(sidbits::CLOCK_FREQ_PAL / (float)kDacUpdateRateHz * (float)kSampleBlockSize);

  Engine() = default;
  DELETE_COPY_MOVE(Engine);

  void Init(Parameters *parameters);
  void Reset();

  void RenderBlock(SampleBuffer::MutableBlock block, const sidbits::RegisterMap &register_map);

  const auto &register_map() const { return cached_registers_; }

  // parameter hooks
  void SystemParameterChanged(SYSTEM parameter) final;
  void GlobalParameterChanged(GLOBAL parameter) final;

protected:
  Parameters *parameters_ = nullptr;

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

  void set_chip_model();
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_ENGINE_H_
