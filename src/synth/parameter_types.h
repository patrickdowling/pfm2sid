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
#ifndef PFM2SID_SYNTH_PARAMETER_TYPES_H_
#define PFM2SID_SYNTH_PARAMETER_TYPES_H_

#include "lfo.h"
#include "parameters.h"
#include "synth.h"

namespace pfm2sid::synth {

struct nibble {};

namespace detail {

template <>
constexpr auto typed_value<bool>(parameter_value_type value)
{
  return !!value;
}

template <>
constexpr auto typed_value<nibble>(parameter_value_type value)
{
  return static_cast<uint8_t>(value);
}

template <>
constexpr auto typed_value<uint16_t>(parameter_value_type value)
{
  return static_cast<uint16_t>(value);
}

template <>
constexpr auto typed_value<sidbits::FILTER_MODE>(parameter_value_type value)
{
  using sidbits::FILTER_MODE;
  constexpr FILTER_MODE filter_modes[] = {
      FILTER_MODE::OFF, FILTER_MODE::LP, FILTER_MODE::BP, FILTER_MODE::HP, FILTER_MODE::NOTCH,
  };
  return filter_modes[value];
}

template <>
constexpr auto typed_value<sidbits::OSC_WAVE>(parameter_value_type value)
{
  // NOTE sort order differs from enum!

  using sidbits::OSC_WAVE;
  auto osc_wave = OSC_WAVE::SILENCE;
  switch (value) {
    case 0: break;
    case 1: osc_wave = OSC_WAVE::TRI; break;
    case 2: osc_wave = OSC_WAVE::SAW; break;
    case 3: osc_wave = OSC_WAVE::PULSE; break;
    case 4: osc_wave = OSC_WAVE::P_T; break;
    case 5: osc_wave = OSC_WAVE::PS_; break;
    case 6: osc_wave = OSC_WAVE::_ST; break;
    case 7: osc_wave = OSC_WAVE::PST; break;
    case 8: osc_wave = OSC_WAVE::NOISE; break;
  }
  return osc_wave;
}

template <>
constexpr auto typed_value<Lfo::SHAPE>(parameter_value_type value)
{
  return static_cast<Lfo::SHAPE>(value);
}

template <>
constexpr auto typed_value<Lfo::ABS>(parameter_value_type value)
{
  return Lfo::ABS{!!value};
}

template <>
constexpr auto typed_value<sidbits::OSC_RING>(parameter_value_type value)
{
  return sidbits::OSC_RING{!!value};
}

template <>
constexpr auto typed_value<sidbits::OSC_SYNC>(parameter_value_type value)
{
  return sidbits::OSC_SYNC{!!value};
}

template <>
constexpr auto typed_value<VOICE_MODE>(parameter_value_type value)
{
  return static_cast<VOICE_MODE>(value);
}

template <>
constexpr auto typed_value<Lfo::SYNC>(parameter_value_type value)
{
  switch (value) {
    case 1: return Lfo::SYNC_NOTE_ON;
  }
  return Lfo::SYNC_NONE;
}

template <>
constexpr auto typed_value<MOD_SRC>(parameter_value_type value)
{
  return static_cast<MOD_SRC>(value);
}

}  // namespace detail
}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_PARAMETER_TYPES_H_
