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
#ifndef PFM2SID_SID_SYNTH_H_
#define PFM2SID_SID_SYNTH_H_

#include "midi/midi_types.h"
#include "sidbits/sidbits.h"
#include "synth/lfo.h"
#include "synth/modulation.h"
#include "synth/parameter_listener.h"
#include "synth/parameter_structs.h"
#include "synth/sid_voice.h"
#include "synth/synth.h"
#include "synth/voice_allocator.h"
#include "util/util_macros.h"

namespace pfm2sid::synth {

class SIDSynth : public ParameterListener {
public:
  static constexpr unsigned kVoiceCount = 3;

  SIDSynth() = default;
  DELETE_COPY_MOVE(SIDSynth);

  void Init(Parameters *parameters, WaveTable *wavetables);
  void Reset();

  const auto &register_map() const { return register_map_; }

  void Update();

  void SetVoiceMode(VOICE_MODE voice_mode, bool force = false);

  void NoteOn(midi::Channel channel, midi::Note note, midi::Velocity velocity);
  void NoteOff(midi::Channel channel, midi::Note note, midi::Velocity velocity);
  void AllNotesOff();
  void Pitchbend(midi::Channel channel, int16_t bend)
  {
    if (channel == midi_channel_) pitch_bend_ = static_cast<float>(bend) / 8192.f;
  }

  bool voice_active(unsigned i) { return voices_[i].active(); }

  // Debug?
  auto bend() const { return pitch_bend_; }

  // ParameterListener hooks
  void GlobalParameterChanged(GLOBAL parameter) final;

  void set_midi_channel(midi::Channel midi_channel)
  {
    if (midi_channel != midi_channel_) {
      AllNotesOff();
      midi_channel_ = midi_channel;
    }
  }

private:
  Parameters *parameters_ = nullptr;

  sidbits::RegisterMap register_map_;

  midi::Channel midi_channel_ = 0;

  SIDVoice voices_[kVoiceCount];
  Lfo lfo_[kNumLfos];

  int32_t filter_key_tracking_ = 0;
  float pitch_bend_ = 0.f;
  ModulationValues modulation_values_;

  VOICE_MODE voice_mode_ = VOICE_MODE::UNISON;

  VoiceAllocatorMono<8> voice_allocator_mono_;
  VoiceAllocator<kVoiceCount, STEAL_STRATEGY::LRU> voice_allocator_poly_;

  NoteStack<8, SORT_NOTES::YES> played_notes_;

  void UpdateModulation();
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SID_SYNTH_H_
