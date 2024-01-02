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
#ifndef PFM2SID_SIDBITS_H_
#define PFM2SID_SIDBITS_H_

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <memory>

#include "misc/fixed_point.h"
#include "util/util_templates.h"

namespace pfm2sid::sidbits {

static constexpr float CLOCK_FREQ_PAL = 985248.f;  // PAL
static constexpr size_t SID_REGISTER_COUNT = 25;

//
// Convert MIDI notes to frequency
//
// Exact note values
uint16_t midi_to_osc_freq(int32_t midi_note);

// 8.24 Note + Fractional part (bitch bend etc.)
// Why 8.16? The value is built from multiple parts (note + modulation, etc.) so this leaves some
// headroom in case of overly excessive modulation. We'd probably also be fine with u824 and some
// care elsewhere.
uint16_t midi_to_osc_freq_fp(util::s816 midi_note);

// Scaled filter response offset
int32_t midi_to_filter_freq(int32_t midi_note, int32_t scale);

// These parameter enums are outside RegisterMap scope else things get a bit long
// Technically we can actually select more than one waveform (just not together with noise).
enum struct OSC_WAVE : uint8_t {
  SILENCE = 0,
  TRI = 0x10,
  SAW = 0x20,
  PULSE = 0x40,
  P_T = PULSE + TRI,
  PS_ = PULSE + SAW,
  _ST = SAW + TRI,
  PST = SAW + SAW + TRI,
  NOISE = 0x80,
};

enum struct FILTER_MODE : uint8_t { OFF = 0x00, HP = 0x40, BP = 0x20, LP = 0x10, NOTCH = LP + HP };

// Yeah, perhaps overkill but avoids mis-assigning parameters in voice_set_control
enum OSC_SYNC : bool {};
enum OSC_RING : bool {};

enum VOICE_INDEX : unsigned { VOICE1, VOICE2, VOICE3 };

// This is just some handy wrappers to set registers.
// TODO range/sanity checks on values
class RegisterMap {
public:
  static constexpr size_t kNumRegisters = SID_REGISTER_COUNT;

  enum REGISTER_OFFSET : unsigned {
    VOICE_FREQ_LO,
    VOICE_FREQ_HI,
    VOICE_PWM_LO,
    VOICE_PWM_HI,
    VOICE_CONTROL,
    VOICE_ENV_AD,
    VOICE_ENV_SR,
    VOICE_REG_COUNT = 7,
    // VOICE 2, 3
    FILTER_CUTOFF_LO = 0x15,
    FILTER_CUTOFF_HI,
    FILTER_RES_FILT,
    FILTER_MODE_VOL,
  };
  static_assert(FILTER_MODE_VOL == 0x18);

  enum VOICE_CONTROL_FLAGS : uint8_t {
    VOICE_CONTROL_GATE = 0x01,
    VOICE_CONTROL_SYNC = 0x02,
    VOICE_CONTROL_RING = 0x40,
    VOICE_CONTROL_WAVE = 0xf0,
  };

  void Reset() { std::fill(registers_.begin(), registers_.end(), 0); }

  void voice_set_freq(VOICE_INDEX voice, uint16_t freq)
  {
    auto *base = voice_register_base(voice);
    base[VOICE_FREQ_LO] = LO(freq);
    base[VOICE_FREQ_HI] = HI(freq);
  }

  uint16_t voice_get_freq(VOICE_INDEX voice) const
  {
    auto *base = voice_register_base(voice);
    return (static_cast<uint16_t>(base[VOICE_FREQ_HI]) << 8) | base[VOICE_FREQ_LO];
  }

  void voice_set_pwm(VOICE_INDEX voice, uint16_t duty_cycle)  // NOTE 12 bits
  {
    auto *base = voice_register_base(voice);
    base[VOICE_PWM_LO] = LO(duty_cycle);
    base[VOICE_PWM_HI] = HI(duty_cycle);
  }

  void voice_set_waveform(VOICE_INDEX voice, OSC_WAVE wave)
  {
    auto *base = voice_register_base(voice);
    base[VOICE_CONTROL] = (base[VOICE_CONTROL] & ~VOICE_CONTROL_WAVE) | static_cast<uint8_t>(wave);
  }

  void voice_set_adsr(VOICE_INDEX voice, uint8_t a, uint8_t d, uint8_t s, uint8_t r)
  {
    auto *base = voice_register_base(voice);
    base[VOICE_ENV_AD] = ((a & 0xf) << 4) | (d & 0xf);
    base[VOICE_ENV_SR] = ((s & 0xf) << 4) | (r & 0xf);
  }

  void voice_set_adsr(VOICE_INDEX voice, const uint8_t *adsr)
  {
    auto *base = voice_register_base(voice);
    base[VOICE_ENV_AD] = ((adsr[0] & 0xf) << 4) | (adsr[1] & 0xf);
    base[VOICE_ENV_SR] = ((adsr[2] & 0xf) << 4) | (adsr[3] & 0xf);
  }

  void voice_set_ring(VOICE_INDEX voice, OSC_RING enable)
  {
    set_flag<VOICE_CONTROL, VOICE_CONTROL_RING>(voice_register_base(voice), enable);
  }

  void voice_set_sync(VOICE_INDEX voice, OSC_SYNC enable)
  {
    set_flag<VOICE_CONTROL, VOICE_CONTROL_SYNC>(voice_register_base(voice), enable);
  }

  void voice_set_gate(VOICE_INDEX voice, bool enable)
  {
    set_flag<VOICE_CONTROL, VOICE_CONTROL_GATE>(voice_register_base(voice), enable);
  }

  void voice_set_control(VOICE_INDEX voice, OSC_WAVE wave, OSC_RING ring, OSC_SYNC sync, bool gate)
  {
    auto *base = voice_register_base(voice);
    uint8_t control = static_cast<uint8_t>(wave);
    if (ring) control |= VOICE_CONTROL_RING;
    if (sync) control |= VOICE_CONTROL_SYNC;
    if (gate) control |= VOICE_CONTROL_GATE;
    base[VOICE_CONTROL] = control;
  }

  enum FILTER_BITS : uint8_t {
    FILTER_FILT_MASK = 0x0f,
    FILTER_RES_MASK = 0xf0,
    FILTER_MODE_MASK = 0xf0,
    FILTER_VOL_MASK = 0x0f,
    FILTER_VOICE3_MUTE = 0x80,
    FILTER_VOICE1 = 0x01,
    FILTER_VOICE2 = 0x02,
    FILTER_VOICE3 = 0x04,
    FILTER_EXT = 0x08,
  };

  void filter_set_freq(uint16_t freq)
  {
    registers_[FILTER_CUTOFF_LO] = (freq & 0x3);
    registers_[FILTER_CUTOFF_HI] = (freq >> 3) & 0xff;
  }

  void filter_set_resonance_enable(uint8_t resonance, bool voice1, bool voice2, bool voice3)
  {
    uint8_t r = (resonance << 4U);
    if (voice1) r |= FILTER_VOICE1;
    if (voice2) r |= FILTER_VOICE2;
    if (voice3) r |= FILTER_VOICE3;
    registers_[FILTER_RES_FILT] = r;
  }

  void filter_set_mode_volume(FILTER_MODE filter_mode, uint8_t volume, bool mute3)
  {
    uint8_t r = static_cast<uint8_t>(filter_mode) | volume;
    if (mute3) r |= FILTER_VOICE3_MUTE;
    registers_[FILTER_MODE_VOL] = r;
  }

  void poke(uint8_t reg, uint8_t value) { registers_[reg] = value; }
  uint8_t peek(uint8_t reg) const { return registers_[reg]; }

  auto begin() const { return registers_.begin(); }
  auto end() const { return registers_.end(); }

private:
  std::array<uint8_t, kNumRegisters> registers_ = {};

  uint8_t *voice_register_base(VOICE_INDEX voice)
  {
    return registers_.data() + voice * REGISTER_OFFSET::VOICE_REG_COUNT;
  }

  const uint8_t *voice_register_base(VOICE_INDEX voice) const
  {
    return registers_.data() + voice * REGISTER_OFFSET::VOICE_REG_COUNT;
  }

  constexpr uint8_t HI(uint16_t v) { return (v >> 8) & 0xff; }
  constexpr uint8_t LO(uint16_t v) { return v & 0xff; }

  template <REGISTER_OFFSET reg, uint8_t bitmask>
  void set_flag(uint8_t *base, bool enable)
  {
    if (enable)
      base[reg] |= bitmask;
    else
      base[reg] &= ~bitmask;
  }
};

}  // namespace pfm2sid::sidbits

ENABLE_ENUM_TO_INDEX(pfm2sid::sidbits::VOICE_INDEX);

#endif  // PFM2SID_SIDBITS_H_
