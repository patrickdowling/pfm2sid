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
#include "parameters.h"

#include <climits>

#include "modulation.h"
#include "parameter_structs.h"
#include "ui/status_icons.h"
#include "util/util_macros.h"
#include "wavetable.h"

// TODO Centralize the min/max range definitions

namespace pfm2sid::synth {

static const char* BOOL_STR[2] = {"off", "ON"};

#define W_P "\005"
#define W_S "\004"
#define W_T "\003"

// NOTE sort order differs from enum!
static const char* OSC_WAVE_STR[] = {
    "off",         W_T " TRI",       W_S " SAW", W_P "PULS", "& " W_P "_" W_T, "& " W_P W_S "_",
    "& _" W_S W_T, "& " W_P W_S W_T, "NOIS",
};

static const char* CHIP_MODEL_STR[] = {"6581", "8580"};

static const char* FILTER_MODE_STR[] = {"off", "LP", "BP", "HP", "NTCH"};

static const char* LFO_SHAPE_STR[] = {"TRI", "SAW", "SQUA", "SINE", "RAMP", "RAND"};

static const char* LFO_SYNC_STR[] = {"none", "note"};

static const char* VOICE_MODE_STR[] = {"poly", "unis"};

static const char* MOD_SRC_STR[] = {"none", "LFO1", "LFO2", "LFO3", "BEND"};
static_assert(ARRAY_SIZE(MOD_SRC_STR) == kNumModulationSrc);

static const char* WAVETABLE_STR[] = {"off", "TBL1", "TBL2", "TBL3", "TBL4"};
static_assert(ARRAY_SIZE(WAVETABLE_STR) == kNumWaveTables + 1);

static constexpr ParameterDesc none_parameter_desc = {"NONE", 0, 0, {}};

static constexpr ParameterDesc system_parameter_descs[] = {
    {"CHAN", 1, 16, SYSTEM::MIDI_CHANNEL, 1},
};

static constexpr ParameterDesc global_parameter_descs[] = {
    {"CHIP", 0, 1, GLOBAL::CHIP_MODEL, 0, CHIP_MODEL_STR},
    {"MODE", 0, 4, GLOBAL::FILTER_MODE, 1, FILTER_MODE_STR},
    {"FREQ", 0, 1024, GLOBAL::FILTER_FREQ, 512},
    {"RES", 0, 15, GLOBAL::FILTER_RES, 0, nullptr, FORMATTER::ZEROISOFF},
    {"VOI1", 0, 1, GLOBAL::FILTER_VOICE1_ENABLE, 0, BOOL_STR},
    {"VOI2", 0, 1, GLOBAL::FILTER_VOICE2_ENABLE, 0, BOOL_STR},
    {"VOI3", 0, 1, GLOBAL::FILTER_VOICE3_ENABLE, 0, BOOL_STR},
    {"3OFF", 0, 1, GLOBAL::FILTER_3OFF, 0, BOOL_STR},
    {"TRAK", -64, 63, GLOBAL::FILTER_KEY_TRACKING, 0, nullptr, FORMATTER::ZEROISOFF},
    {"NOTE", 0, 1, GLOBAL::FILTER_KEY_TRACK_NOTE},
    {RIGHT_ARROW_STR "frq", 0, util::enum_count<MOD_SRC>() - 1, GLOBAL::FILTER_FREQ_MOD_SRC, 0,
     MOD_SRC_STR},
    {"Dpth", kModDepthMin, kModDepthMax, GLOBAL::FILTER_FREQ_MOD_DEPTH},
    {RIGHT_ARROW_STR "res", 0, util::enum_count<MOD_SRC>() - 1, GLOBAL::FILTER_RES_MOD_SRC, 0,
     MOD_SRC_STR},
    {"Dpth", kModDepthMin, kModDepthMax, GLOBAL::FILTER_RES_MOD_DEPTH},
    {"VOL", 0, 15, GLOBAL::VOLUME, 15},
    {"VOIC", 0, 1, GLOBAL::VOICE_MODE, 0, VOICE_MODE_STR},
};

static constexpr ParameterDesc voice_parameter_descs[] = {
    {"OCT", -3, 3, VOICE::TUNE_OCTAVE},
    {"SEMI", -11, 11, VOICE::TUNE_SEMITONE},
    {"FINE", -128, 127, VOICE::TUNE_FINE},
    {"GLID", 0, 127, VOICE::GLIDE_RATE, 0, nullptr, FORMATTER::ZEROISOFF},
    {"WAVE", 0, static_cast<int32_t>(sidbits::OSC_WAVE::NOISE) >> 4, VOICE::OSC_WAVE, 2,
     OSC_WAVE_STR},
    {"PWM", 0, 4095, VOICE::OSC_PWM, 2048},
    {"RING", 0, 1, VOICE::OSC_RING, 0, BOOL_STR},
    {"SYNC", 0, 1, VOICE::OSC_SYNC, 0, BOOL_STR},
    {"ATT", 0, 15, VOICE::ENV_A, 0},
    {"DEC", 0, 15, VOICE::ENV_D, 0},
    {"SUS", 0, 15, VOICE::ENV_S, 15},
    {"REL", 0, 15, VOICE::ENV_R, 9},
    {RIGHT_ARROW_STR "frq", 0, util::enum_count<MOD_SRC>() - 1, VOICE::FREQ_MOD_SRC, 0,
     MOD_SRC_STR},
    {"Dpth", kModDepthMin, kModDepthMax, VOICE::FREQ_MOD_DEPTH},
    {RIGHT_ARROW_STR "PWM", 0, util::enum_count<MOD_SRC>() - 1, VOICE::PWM_MOD_SRC, 0, MOD_SRC_STR},
    {"Dpth", kModDepthMin, kModDepthMax, VOICE::PWM_MOD_DEPTH},
    {"WTBL", 0, kNumWaveTables, VOICE::WAVETABLE_IDX, 0, WAVETABLE_STR},
    {"RATE", WaveTableScanner::kRateMin, WaveTableScanner::kRateMax, VOICE::WAVETABLE_RATE, 63},
};

static constexpr ParameterDesc lfo_parameter_descs[] = {
    {"RATE", 0, 127, LFO::RATE, 63},      {"SHAP", 0, 5, LFO::SHAPE, 0, LFO_SHAPE_STR},
    {"PHA", 0, 127, LFO::PHASE, 0},       {"SYNC", 0, 1, LFO::SYNC, 0, LFO_SYNC_STR},
    {"ABS", 0, 1, LFO::ABS, 0, BOOL_STR},
};

static_assert(ARRAY_SIZE(system_parameter_descs) == kNumSystemParameters);
static_assert(ARRAY_SIZE(global_parameter_descs) == kNumGlobalParameters);
static_assert(ARRAY_SIZE(voice_parameter_descs) == kNumVoiceParameters);
static_assert(ARRAY_SIZE(lfo_parameter_descs) == kNumLfoParameters);

/*static*/
const ParameterDesc* ParameterDesc::Find(const ParameterRef parameter)
{
  switch (parameter.type) {
    case PARAMETER_SCOPE::SYSTEM:
      return &system_parameter_descs[util::enum_to_i(parameter.system_param)];
      break;
    case PARAMETER_SCOPE::GLOBAL:
      return &global_parameter_descs[util::enum_to_i(parameter.global_param)];
      break;
    case PARAMETER_SCOPE::VOICE:
      return &voice_parameter_descs[util::enum_to_i(parameter.voice_param)];
      break;
    case PARAMETER_SCOPE::LFO:
      return &lfo_parameter_descs[util::enum_to_i(parameter.lfo_param)];
      break;
    case PARAMETER_SCOPE::NONE: break;
  }
  return &none_parameter_desc;
}

void Parameters::Reset()
{
  detail::ResetArray(global_parameters_);
  detail::ResetArrays(voice_parameters_);
  detail::ResetArrays(lfo_parameters_);
}

void SystemParameters::Reset()
{
  detail::ResetArray(system_parameters_);
}

}  // namespace pfm2sid::synth
