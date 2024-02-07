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
#include "sid_voice.h"

#include "modulation.h"
#include "parameter_structs.h"
#include "parameter_types.h"

// ENABLE_WCONVERSION();

// NOTES
// To ensure there's an off/on for gate, use a trigger delay; this should be rare since the MIDI
// messages are few compared to updates? Or maybe just a re-trigger state might suffice.
//
// - Leave the frequency and waveform after GATE_LOW, otherwise a Release might be cut off

namespace pfm2sid::synth {

void SIDVoice::Init(sidbits::VOICE_INDEX voice_index, const Parameters *parameters,
                    const WaveTable *wavetables)
{
  sid_voice_ = voice_index;
  parameters_ = parameters;
  wavetables_ = wavetables;
}

void SIDVoice::Reset()
{
  // note_ = midi::INVALID_NOTE;
  velocity_ = 0;
  gate_state_ = GATE_FALLING;  // TODO This is kind of hack to ensure we clear the gate bit
  glide_.Reset();
  wavetable_scanner_.Reset();
}

void SIDVoice::NoteOn(midi::Note note, midi::Velocity velocity, bool glide)
{
  note_ = note;
  velocity_ = velocity;
  gate_state_ = GATE_RISING;

  glide_.Init(note, glide ? parameters_->get<VOICE::GLIDE_RATE>(parameter_voice_).value() : 0);

  // Get ADSR values here
  adsr_[0] = parameters_->get<VOICE::ENV_A, nibble>(parameter_voice_);
  adsr_[1] = parameters_->get<VOICE::ENV_D, nibble>(parameter_voice_);
  adsr_[2] = parameters_->get<VOICE::ENV_S, nibble>(parameter_voice_);  // TODO velocity?
  adsr_[3] = parameters_->get<VOICE::ENV_R, nibble>(parameter_voice_);

  // We'll only initialze the wavetable on note on, but change the rate later
  auto wavetable_idx = parameters_->get<VOICE::WAVETABLE_IDX>(parameter_voice_).value();
  if (wavetable_idx) {
    wavetable_scanner_.set_rate(parameters_->get<VOICE::WAVETABLE_RATE>(parameter_voice_).value());
    wavetable_scanner_.SetSource(&wavetables_[wavetable_idx - 1]);
  } else {
    wavetable_scanner_.Reset();
  }
}

void SIDVoice::NoteOff(midi::Note note)
{
  // This turns off the gate, but maintains the voice as active until the next Update
  if (gate_state_ && note == note_) { gate_state_ = GATE_FALLING; }
}

void SIDVoice::Update(sidbits::RegisterMap &register_map, const ModulationValues &modulation_values)
{
  if (active()) {
    WaveTable::Entry wte;
    if (wavetable_scanner_.active()) {
      wavetable_scanner_.set_rate(
          parameters_->get<VOICE::WAVETABLE_RATE>(parameter_voice_).value());
      wte = wavetable_scanner_.Update();
    }

    // Apply glide to the note value, then apply modulation, then get frequency
    // This seems easier than doing it in frequency units?
    auto note = glide_.Update();

    // TODO it's a bit unclear if we should add octave/transpose to the glide target?
    auto note_offset = parameters_->get<VOICE::TUNE_OCTAVE>(parameter_voice_).value() * 12 +
                       parameters_->get<VOICE::TUNE_SEMITONE>(parameter_voice_).value();
    if (wavetable_scanner_.is_enabled<WaveTable::TRANSPOSE>()) note_offset += wte.transpose;

    note.add_integral(note_offset);

    // note is a fixed-point value, so we need to ensure fine is compatible.
    int32_t fine_offset = modulation_values.get(
        parameters_->get<VOICE::FREQ_MOD_SRC, MOD_SRC>(parameter_voice_),
        parameters_->get<VOICE::FREQ_MOD_DEPTH>(parameter_voice_).value(), 256.f);
    fine_offset += parameters_->get<VOICE::TUNE_FINE>(parameter_voice_).value();
    fine_offset += modulation_values.get(MOD_SRC::PITCH_BEND, 256, 512.f);

    note.add_fractional(fine_offset << 8);

    uint16_t freq = sidbits::midi_to_osc_freq_fp(note);

    auto gate_state = gate_state_;
    if (GATE_RISING == gate_state) {
      register_map.voice_set_adsr(sid_voice_, adsr_.data());
      gate_state = GATE_HIGH;
    } else if (GATE_FALLING == gate_state) {
      gate_state = GATE_LOW;
      note_ = midi::INVALID_NOTE;
    }

    register_map.voice_set_freq(sid_voice_, freq);

    auto pwm_mod = modulation_values.get(
        parameters_->get<VOICE::PWM_MOD_SRC, MOD_SRC>(parameter_voice_),
        parameters_->get<VOICE::PWM_MOD_DEPTH>(parameter_voice_).value(), 2048.f);
    auto pwm = parameters_->get<VOICE::OSC_PWM>(parameter_voice_).modulate_value<uint16_t>(pwm_mod);
    register_map.voice_set_pwm(sid_voice_, pwm);

    sidbits::OSC_WAVE wave;
    if (wavetable_scanner_.is_enabled<WaveTable::WAVEFORM>())
      wave = wte.waveform;
    else
      wave = parameters_->get<VOICE::OSC_WAVE, sidbits::OSC_WAVE>(parameter_voice_);

    register_map.voice_set_control(
        sid_voice_, wave, parameters_->get<VOICE::OSC_RING, sidbits::OSC_RING>(parameter_voice_),
        parameters_->get<VOICE::OSC_SYNC, sidbits::OSC_SYNC>(parameter_voice_),
        GATE_HIGH == gate_state);

    gate_state_ = gate_state;
  }
}

}  // namespace pfm2sid::synth
