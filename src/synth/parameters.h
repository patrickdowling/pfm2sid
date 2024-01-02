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
#ifndef PFM2SID_SYNTH_PARAMETERS_H_
#define PFM2SID_SYNTH_PARAMETERS_H_

#include "sidbits/sidbits.h"
#include "synth.h"
#include "util/util_templates.h"

namespace pfm2sid::synth {

using parameter_enum_type = unsigned;
using parameter_value_type = int32_t;

enum struct PARAMETER_SCOPE { NONE, SYSTEM, GLOBAL, VOICE, LFO };

// Trying to cover all the parameters with a single enum becomes a bit tedious with multiple voices
// and things like lfos and modulation slots, which have multiple instances that all have the same
// basic parameters. So instead, there's a bit of complexity added to the "storage" layer to handle
// this.
//
// Since most of the parameter lookups are templated, the offset and array switch becomes static so
// the overall runtime cost should be low?
//
// We can try and catch mis-use through static_assert but it's still all somewhat convoluted.
//
// TODO The basic question eventually becomes, why the enums at all?

enum struct SYSTEM : parameter_enum_type { MIDI_CHANNEL, LAST };

enum struct GLOBAL : parameter_enum_type {
  CHIP_MODEL,
  FILTER_MODE,
  FILTER_FREQ,
  FILTER_RES,
  FILTER_VOICE1_ENABLE,  // It is perhaps debatable whether this should be a VOICE setting
  FILTER_VOICE2_ENABLE,
  FILTER_VOICE3_ENABLE,
  FILTER_3OFF,
  FILTER_KEY_TRACKING,
  FILTER_KEY_TRACK_NOTE,  // highest, lowest, last
  FILTER_FREQ_MOD_SRC,
  FILTER_FREQ_MOD_DEPTH,
  FILTER_RES_MOD_SRC,
  FILTER_RES_MOD_DEPTH,
  VOLUME,
  VOICE_MODE,
  LAST,
};

enum struct VOICE : parameter_enum_type {
  TUNE_OCTAVE,
  TUNE_SEMITONE,
  TUNE_FINE,
  GLIDE_RATE,
  OSC_WAVE,
  OSC_PWM,
  OSC_RING,
  OSC_SYNC,
  ENV_A,
  ENV_D,
  ENV_S,
  ENV_R,
  FREQ_MOD_SRC,
  FREQ_MOD_DEPTH,
  PWM_MOD_SRC,
  PWM_MOD_DEPTH,
  WAVETABLE_IDX,
  WAVETABLE_RATE,
  LAST,
};

enum struct LFO : parameter_enum_type { RATE, SHAPE, PHASE, SYNC, ABS, LAST };

static constexpr auto kNumSystemParameters = util::enum_count<SYSTEM>();
static constexpr auto kNumGlobalParameters = util::enum_count<GLOBAL>();
static constexpr auto kNumVoiceParameters = util::enum_count<VOICE>();
static constexpr auto kNumLfoParameters = util::enum_count<LFO>();

namespace detail {

// Convert generic ParameterValue into some (stronger) typed value
// TODO Should this be typed on the PARAMETER enum(s)? That may make more sense if we can remove
// some of the parameter duplication for lfos and voices but ends up being more specializations.
//
// An advantage there (or a disadvantage to this template) is that duplicates might be avoided (e.g.
// uint8_t and midi::Channel)
template <typename T>
constexpr auto typed_value(parameter_value_type value);

}  // namespace detail

}  // namespace pfm2sid::synth

ENABLE_ENUM_TO_INDEX(pfm2sid::synth::SYSTEM);
ENABLE_ENUM_TO_INDEX(pfm2sid::synth::GLOBAL);
ENABLE_ENUM_TO_INDEX(pfm2sid::synth::VOICE);
ENABLE_ENUM_TO_INDEX(pfm2sid::synth::LFO);

#endif  // PFM2SID_SYNTH_PARAMETERS_H_
