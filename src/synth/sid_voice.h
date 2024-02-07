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
#ifndef PFM2SID_SID_VOICE_H_
#define PFM2SID_SID_VOICE_H_

#include "glide.h"
#include "midi/midi_types.h"
#include "modulation.h"
#include "sid.h"
#include "sidbits/sidbits.h"
#include "util/util_macros.h"
#include "wavetable.h"

namespace pfm2sid::synth {

class Parameters;

class SIDVoice {
public:
  enum GATE_STATE { GATE_LOW, GATE_RISING, GATE_HIGH, GATE_FALLING };

  SIDVoice() = default;
  DELETE_COPY_MOVE(SIDVoice);

  void Init(sidbits::VOICE_INDEX voice_index, const Parameters *parameter,
            const WaveTable *wavtables);

  void Reset();

  void set_parameter_voice(sidbits::VOICE_INDEX voice) { parameter_voice_ = voice; }

  void NoteOn(midi::Note note, midi::Velocity velocity, bool glide = false);
  void NoteOff(midi::Note note);

  void Update(sidbits::RegisterMap &register_map, const ModulationValues &modulation_values);

  bool active() const { return note_ != 0xff; }

  auto sid_voice() const { return sid_voice_; }

private:
  sidbits::VOICE_INDEX sid_voice_ = sidbits::VOICE1;
  sidbits::VOICE_INDEX parameter_voice_ = sidbits::VOICE1;

  const Parameters *parameters_ = nullptr;
  const WaveTable *wavetables_ = nullptr;

  midi::Note note_ = midi::INVALID_NOTE;
  uint8_t velocity_ = 0;
  std::array<uint8_t, 4> adsr_;
  GATE_STATE gate_state_ = GATE_LOW;

  Glide glide_;
  WaveTableScanner wavetable_scanner_;
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SID_VOICE_H_
