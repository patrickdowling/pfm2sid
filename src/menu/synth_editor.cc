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
#include "synth_editor.h"

#include <cinttypes>

#include "menu_util.h"
#include "pfm2sid.h"
#include "pfm2sid_stats.h"
#include "synth/engine.h"
#include "synth/patch.h"
#include "synth/sid_synth.h"
#include "ui/display.h"

namespace pfm2sid {

extern synth::SystemParameters system_parameters;
extern synth::Patch current_patch;
extern synth::Engine engine;
extern synth::SIDSynth sid_synth_;

namespace synth {

static constexpr EditorPageDef editor_page_defs[] = {
    {},  // NONE
    {},  // HEXDUMP
    {"Info"},
    {"Stats"},
    {"Tuning",
     PARAMETER_SCOPE::VOICE,
     {VOICE::TUNE_OCTAVE, VOICE::TUNE_SEMITONE, VOICE::TUNE_FINE, VOICE::GLIDE_RATE}},
    {"Waveform",
     PARAMETER_SCOPE::VOICE,
     {VOICE::OSC_WAVE, VOICE::OSC_PWM, VOICE::OSC_RING, VOICE::OSC_SYNC}},
    {"Envelope", PARAMETER_SCOPE::VOICE, {VOICE::ENV_A, VOICE::ENV_D, VOICE::ENV_S, VOICE::ENV_R}},
    {"Modulation",
     PARAMETER_SCOPE::VOICE,
     {VOICE::FREQ_MOD_SRC, VOICE::FREQ_MOD_DEPTH, VOICE::PWM_MOD_SRC, VOICE::PWM_MOD_DEPTH}},
    {"Wavetable", PARAMETER_SCOPE::VOICE, {VOICE::WAVETABLE_IDX, VOICE::WAVETABLE_RATE, {}, {}}},
    {"Routing",
     PARAMETER_SCOPE::GLOBAL,
     {GLOBAL::FILTER_VOICE1_ENABLE, GLOBAL::FILTER_VOICE2_ENABLE, GLOBAL::FILTER_VOICE3_ENABLE,
      GLOBAL::FILTER_3OFF}},
    {"Filter",
     PARAMETER_SCOPE::GLOBAL,
     {GLOBAL::FILTER_MODE, GLOBAL::FILTER_FREQ, GLOBAL::FILTER_RES, GLOBAL::FILTER_KEY_TRACKING}},
    {"Filter mod",
     PARAMETER_SCOPE::GLOBAL,
     {GLOBAL::FILTER_FREQ_MOD_SRC, GLOBAL::FILTER_FREQ_MOD_DEPTH, GLOBAL::FILTER_RES_MOD_SRC,
      GLOBAL::FILTER_RES_MOD_DEPTH}},
    {"Shape", PARAMETER_SCOPE::LFO, {LFO::SHAPE, LFO::RATE, LFO::PHASE, LFO::ABS}},
    {"Control", PARAMETER_SCOPE::LFO, {LFO::SYNC, {}, {}, {}}},
    {"System",
     PARAMETER_SCOPE::SYSTEM,
     {GLOBAL::VOICE_MODE, SYSTEM::MIDI_CHANNEL, GLOBAL::CHIP_MODEL, GLOBAL::VOLUME}},
};
static_assert(ARRAY_SIZE(editor_page_defs) == util::enum_count<EDITOR_PAGE>());

static constexpr EDITOR_PAGE menu_levels[][5] = {
    {EDITOR_PAGE::EDIT_VOICE_OSC, EDITOR_PAGE::EDIT_VOICE_TUNE, EDITOR_PAGE::NONE},
    {EDITOR_PAGE::EDIT_VOICE_ENV, EDITOR_PAGE::EDIT_VOICE_WAVETABLE, EDITOR_PAGE::NONE},
    {EDITOR_PAGE::EDIT_FILTER, EDITOR_PAGE::EDIT_FILTER_VOICES, EDITOR_PAGE::NONE},
    {EDITOR_PAGE::EDIT_VOICE_MOD, EDITOR_PAGE::EDIT_FILTER_MOD, EDITOR_PAGE::NONE},
    {EDITOR_PAGE::EDIT_LFO, EDITOR_PAGE::EDIT_LFO_EXT, EDITOR_PAGE::NONE},
    {EDITOR_PAGE::NONE},
    {EDITOR_PAGE::EDIT_MISC, EDITOR_PAGE::HEXDUMP, EDITOR_PAGE::INFO, EDITOR_PAGE::STATS,
     EDITOR_PAGE::NONE},
};

void SIDSynthEditor::HandleMenuEvent(MENU_EVENT menu_event)
{
  if (MENU_EVENT::ENTER == menu_event) {
    engine.Reset();
    if (menu_level_ < 0) SetMenuLevel(0);
  }
}
void SIDSynthEditor::HandleEvent(const Event &event)
{
  switch (event.type) {
    case EVENT_BUTTON_PRESS:
      switch (event.control) {
        case CONTROL::SWITCH1: SetMenuLevel(0); break;
        case CONTROL::SWITCH2: SetMenuLevel(1); break;
        case CONTROL::SWITCH3: SetMenuLevel(2); break;
        case CONTROL::SWITCH4: SetMenuLevel(3); break;
        case CONTROL::SWITCH5: SetMenuLevel(4); break;
        case CONTROL::SWITCH6: CycleVoiceOrLfo(); break;
        case CONTROL::SWITCH7: SetMenuLevel(6); break;
        default: break;
      }
      break;
    case EVENT_BUTTON_LONG_PRESS:
      switch (event.control) {
        case CONTROL::SWITCH7: SetMenuLevel(6, true); break;
        default: break;
      }
    case EVENT_BUTTON_RELEASE: break;
    case EVENT_ENCODER:
      if (util::enum_to_i(editor_page_) >= util::enum_to_i(EDITOR_PAGE::EDIT_VOICE_TUNE)) {
        auto idx = encoder_index(event.control);
        if (values_[idx]) {
          if (values_[idx]->change_value(event.value)) {
            auto ref = values_[idx]->ref();
            if (ref.is_system()) {
              for (auto l : listeners_) l->SystemParameterChanged(ref.system_param);
            } else if (ref.is_global()) {
              for (auto l : listeners_) l->GlobalParameterChanged(ref.global_param);
            }
          }
        }
      }
      break;
    default: break;
  }
}

// There is definitely room for improvement here, we don't need to redraw most of the screen since
// it's static. And the formatting is horribly ineffecient.
void SIDSynthEditor::UpdateDisplay() const
{
  if (editor_page_ == EDITOR_PAGE::HEXDUMP) {
    menu::HexdumpRegisters(engine.register_map());
    return;
  }

  if (editor_page_ == EDITOR_PAGE::INFO) {
    display.Fmt(2, "%4.1fx%" PRIu32 " dt=%u",
                PRINT_F32(static_cast<float>(kDacUpdateRateHz) / 1000.f), kSampleBlockSize,
                Engine::clock_delta_t);
    display.Fmt(3, "%4.1fKhz", PRINT_F32(kModulatorUpdateRateHz));
    return;
  }

  if (editor_page_ == EDITOR_PAGE::STATS) {
    constexpr uint32_t blk_max =
        static_cast<uint32_t>((1000.f * 1000.f) / (static_cast<float>(kDacUpdateRateHz) /
                                                   static_cast<float>(kSampleBlockSize)) +
                              .5f);
    display.Fmt(0, "blk %4" PRIu32 "/%" PRIu32 "us", stats::render_block_cycles.value_in_us(),
                blk_max);
    display.Fmt(1, "sid %4" PRIu32 "us", stats::sid_clock_cycles.value_in_us());
    // display.Fmt(3, "%.3f", sid_synth_.bend());
    return;
  }
  {
    const auto &page_def = editor_page_defs[util::enum_to_i(editor_page_)];

    display.Fmt(0, "%02d %s", current_patch.number(), current_patch.name());

    char buf[20] = {0};
    switch (page_def.parameter_type) {
      case PARAMETER_SCOPE::NONE:
      case PARAMETER_SCOPE::SYSTEM:
      case PARAMETER_SCOPE::GLOBAL: snprintf(buf, sizeof(buf), "%s", page_def.title); break;
      case PARAMETER_SCOPE::VOICE:
        snprintf(buf, sizeof(buf), "VOI%d/%d %s", voice_index_ + 1, 3, page_def.title);
        break;
      case PARAMETER_SCOPE::LFO:
        snprintf(buf, sizeof(buf), "LFO%d/%d %s", lfo_index_ + 1, 3, page_def.title);
        break;
    }

    display.Fmt(1, "%c%-18s%c", navigation_[0], buf, navigation_[1]);
    for (int i = 0; i < 4; ++i) {
      auto value = values_[i];
      if (value) {
        snprintf(name_buf_ + (i * 5), 6, "%5s", value->name());
        value->Fmt(value_buf_ + (i * 5));
      } else {
        memset(name_buf_ + (i * 5), ' ', 5);
        memset(value_buf_ + (i * 5), ' ', 5);
      }

      display.Write(2, name_buf_);
      display.Write(3, value_buf_);
    }
  }
}

void SIDSynthEditor::CycleVoiceOrLfo()
{
  auto page_def = editor_page_defs[static_cast<unsigned>(editor_page_)];

  if (page_def.parameter_type == PARAMETER_SCOPE::VOICE) {
    unsigned voice_index = voice_index_;
    if (voice_index < sidbits::VOICE3)
      ++voice_index;
    else
      voice_index = sidbits::VOICE1;
    voice_index_ = static_cast<sidbits::VOICE_INDEX>(voice_index);
    SetPage(editor_page_, true);
  } else if (page_def.parameter_type == PARAMETER_SCOPE::LFO) {
    unsigned lfo_index = lfo_index_;
    if (lfo_index < LFO_INDEX::LFO3)
      ++lfo_index;
    else
      lfo_index = LFO_INDEX::LFO1;
    lfo_index_ = static_cast<LFO_INDEX>(lfo_index);
    SetPage(editor_page_, true);
  }
}

void SIDSynthEditor::SetMenuLevel(int level, bool force)
{
  if (force || level != menu_level_) {
    menu_level_ = level;
    menu_subpage_ = 0;
  } else {
    auto subpage = menu_subpage_ + 1;
    if (EDITOR_PAGE::NONE == menu_levels[level][subpage]) subpage = 0;
    menu_subpage_ = subpage;
  }

  SetPage(menu_levels[menu_level_][menu_subpage_], force);
}

void SIDSynthEditor::SetPage(EDITOR_PAGE editor_page, bool force)
{
  if (editor_page == editor_page_ && !force) return;

  editor_page_ = editor_page;
  auto page_def = editor_page_defs[static_cast<unsigned>(editor_page)];
  for (int i = 0; i < 4; ++i) {
    auto &ref = page_def.parameters[i];
    switch (ref.type) {
      case PARAMETER_SCOPE::SYSTEM: values_[i] = system_parameters.mutable_value(ref); break;
      case PARAMETER_SCOPE::GLOBAL: values_[i] = current_patch.parameters.mutable_value(ref); break;
      case PARAMETER_SCOPE::VOICE:
        values_[i] = current_patch.parameters.mutable_value(ref, voice_index_);
        break;
      case PARAMETER_SCOPE::LFO:
        values_[i] = current_patch.parameters.mutable_value(ref, lfo_index_);
        break;
      default: values_[i] = nullptr;
    }
  }

  navigation_[0] = menu_subpage_ ? LEFT_ARROW_CHAR : ' ';
  navigation_[1] =
      menu_levels[menu_level_][menu_subpage_ + 1] != EDITOR_PAGE::NONE ? RIGHT_ARROW_CHAR : ' ';

  display.Clear();
}

}  // namespace synth
}  // namespace pfm2sid
