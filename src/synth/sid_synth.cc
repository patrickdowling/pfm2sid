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
#include "sid_synth.h"

#include "misc/platform.h"
#include "parameter_types.h"
#include "stm32x/stm32x_debug.h"

ENABLE_WCONVERSION();

namespace pfm2sid::synth {

void SIDSynth::Init(Parameters *parameters)
{
  parameters_ = parameters;

  voices_[0].Init(sidbits::VOICE1, parameters);
  voices_[1].Init(sidbits::VOICE2, parameters);
  voices_[2].Init(sidbits::VOICE3, parameters);

  for (auto &lfo : lfo_) lfo.Init(0, 0.f);

  SetVoiceMode(parameters->get<GLOBAL::VOICE_MODE, VOICE_MODE>(), true);

  Reset();
}

void SIDSynth::Reset()
{
  register_map_.Reset();

  for (auto &v : voices_) v.Reset();
  for (auto &lfo : lfo_) lfo.Reset(0.f);

  filter_key_tracking_ = 0;
  pitch_bend_ = 0.f;

  voice_allocator_mono_.Clear();
  voice_allocator_poly_.Clear();
  played_notes_.Clear();
}

void SIDSynth::GlobalParameterChanged(GLOBAL parameter)
{
  switch (parameter) {
    case GLOBAL::VOICE_MODE:
      SetVoiceMode(parameters_->get<GLOBAL::VOICE_MODE, VOICE_MODE>());
      break;
    default: break;
  }
}

void SIDSynth::SetVoiceMode(VOICE_MODE voice_mode, bool force /*= false*/)
{
  if (voice_mode_ != voice_mode || force) {
    for (auto &v : voices_) v.Reset();

    switch (voice_mode) {
      case VOICE_MODE::UNISON:
        voice_allocator_mono_.Clear();
        for (auto &v : voices_) v.set_parameter_voice(v.sid_voice());
        break;
      case VOICE_MODE::POLY:
        voice_allocator_poly_.Clear();
        for (auto &v : voices_) v.set_parameter_voice(sidbits::VOICE1);
        break;
    }
    voice_mode_ = voice_mode;
  }
}

void SIDSynth::NoteOn([[maybe_unused]] midi::Channel channel, midi::Note note,
                      midi::Velocity velocity)
{
  bool note_on = false;
  if (velocity) {
    switch (voice_mode_) {
      case VOICE_MODE::POLY: {
        auto v = voice_allocator_poly_.NoteOn(note, velocity);
        if (v) {
          voices_[v.value()].NoteOn(note, velocity);
          note_on = true;
        }
      } break;
      case VOICE_MODE::UNISON: {
        bool glide = voice_allocator_mono_.size() > 0;
        voice_allocator_mono_.NoteOn(note, velocity);
        for (auto &v : voices_) v.NoteOn(note, velocity, glide);
        note_on = true;
      } break;
    }
  } else {
    NoteOff(channel, note, velocity);
  }

  if (note_on) {
    played_notes_.NoteOn(note, velocity);
    if (Lfo::SYNC_NOTE_ON == parameters_->get<LFO1, LFO::SYNC, Lfo::SYNC>()) lfo_[0].Reset(0);
    if (Lfo::SYNC_NOTE_ON == parameters_->get<LFO2, LFO::SYNC, Lfo::SYNC>()) lfo_[1].Reset(0);
    if (Lfo::SYNC_NOTE_ON == parameters_->get<LFO3, LFO::SYNC, Lfo::SYNC>()) lfo_[2].Reset(0);
  }
}

void SIDSynth::NoteOff([[maybe_unused]] midi::Channel channel, midi::Note note,
                       midi::Velocity /*velocity*/)
{
  played_notes_.NoteOff(note);
  switch (voice_mode_) {
    case VOICE_MODE::POLY: {
      auto v = voice_allocator_poly_.NoteOff(note);
      if (v) voices_[v.value()].NoteOff(note);
    } break;
    case VOICE_MODE::UNISON: {
      for (auto &v : voices_) v.NoteOff(note);
      voice_allocator_mono_.NoteOff(note);
      if (voice_allocator_mono_.size()) {
        auto active_note = voice_allocator_mono_.active_note();
        played_notes_.NoteOn(active_note.note, active_note.velocity);
        for (auto &v : voices_) v.NoteOn(active_note.note, active_note.velocity, true);
        // TODO Should this resync the LFOs?
      }
    } break;
  }
}

void SIDSynth::AllNotesOff()
{
  for (auto &v : voices_) v.Reset();

  filter_key_tracking_ = 0;
  pitch_bend_ = 0.f;

  voice_allocator_mono_.Clear();
  voice_allocator_poly_.Clear();
  played_notes_.Clear();
}

void SIDSynth::Update()
{
  UpdateModulation();

  auto res_mod =
      modulation_values_.get(parameters_->get<GLOBAL::FILTER_RES_MOD_SRC, MOD_SRC>(),
                             parameters_->get<GLOBAL::FILTER_RES_MOD_DEPTH>().value(), 16.f);
  auto filter_res = parameters_->get<GLOBAL::FILTER_RES>().modulate_value<nibble>(res_mod);

  if (VOICE_MODE::POLY == voice_mode_)
    register_map_.filter_set_resonance_enable(
        filter_res, parameters_->get<GLOBAL::FILTER_VOICE1_ENABLE, bool>(),
        parameters_->get<GLOBAL::FILTER_VOICE1_ENABLE, bool>(),
        parameters_->get<GLOBAL::FILTER_VOICE1_ENABLE, bool>());
  else
    register_map_.filter_set_resonance_enable(
        filter_res, parameters_->get<GLOBAL::FILTER_VOICE1_ENABLE, bool>(),
        parameters_->get<GLOBAL::FILTER_VOICE2_ENABLE, bool>(),
        parameters_->get<GLOBAL::FILTER_VOICE3_ENABLE, bool>());

  auto f_mod =
      modulation_values_.get(parameters_->get<GLOBAL::FILTER_FREQ_MOD_SRC, MOD_SRC>(),
                             parameters_->get<GLOBAL::FILTER_FREQ_MOD_DEPTH>().value(), 1024.f);

  // TODO This is a fairly abrupt transition
  auto filter_key_tracking = filter_key_tracking_;
  if (played_notes_.size()) {
    filter_key_tracking =
        sidbits::midi_to_filter_freq(played_notes_.sorted_notes().back(),
                                     parameters_->get<GLOBAL::FILTER_KEY_TRACKING>().value());
    filter_key_tracking_ = filter_key_tracking;
  }
  f_mod += filter_key_tracking;
  auto ff = parameters_->get<GLOBAL::FILTER_FREQ>().modulate_value<uint16_t>(f_mod);
  register_map_.filter_set_freq(ff);

  register_map_.filter_set_mode_volume(
      parameters_->get<GLOBAL::FILTER_MODE, sidbits::FILTER_MODE>(),
      parameters_->get<GLOBAL::VOLUME, nibble>(), parameters_->get<GLOBAL::FILTER_3OFF, bool>());

  for (auto &voice : voices_) { voice.Update(register_map_, modulation_values_); }
}

void SIDSynth::UpdateModulation()
{
  modulation_values_.mutable_src<MOD_SRC::LFO1>() = lfo_[0].Update(
      parameters_->get<LFO1, LFO::RATE>().value(), parameters_->get<LFO1, LFO::SHAPE, Lfo::SHAPE>(),
      parameters_->get<LFO1, LFO::ABS, Lfo::ABS>());
  modulation_values_.mutable_src<MOD_SRC::LFO2>() = lfo_[1].Update(
      parameters_->get<LFO2, LFO::RATE>().value(), parameters_->get<LFO2, LFO::SHAPE, Lfo::SHAPE>(),
      parameters_->get<LFO2, LFO::ABS, Lfo::ABS>());
  modulation_values_.mutable_src<MOD_SRC::LFO3>() = lfo_[1].Update(
      parameters_->get<LFO3, LFO::RATE>().value(), parameters_->get<LFO3, LFO::SHAPE, Lfo::SHAPE>(),
      parameters_->get<LFO3, LFO::ABS, Lfo::ABS>());

  modulation_values_.mutable_src<MOD_SRC::PITCH_BEND>() = pitch_bend_;
}

}  // namespace pfm2sid::synth
