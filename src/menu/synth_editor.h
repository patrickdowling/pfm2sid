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
#ifndef PFM2SID_MENU_SYNTH_EDITOR_H_
#define PFM2SID_MENU_SYNTH_EDITOR_H_

#include <array>

#include "midi/midi_parser.h"
#include "misc/static_stack.h"
#include "synth/parameter_listener.h"
#include "synth/parameter_structs.h"
#include "ui/menu.h"

namespace pfm2sid::synth {

enum struct EDITOR_PAGE {
  NONE,
  HEXDUMP,
  INFO,
  STATS,
  EDIT_VOICE_TUNE,
  EDIT_VOICE_OSC,
  EDIT_VOICE_ENV,
  EDIT_VOICE_MOD,
  EDIT_VOICE_WAVETABLE,
  EDIT_FILTER_VOICES,
  EDIT_FILTER,
  EDIT_FILTER_MOD,
  EDIT_LFO,
  EDIT_LFO_EXT,
  EDIT_MISC,
  LAST
};

struct EditorPageDef {
  const char *const title = nullptr;
  PARAMETER_SCOPE parameter_type = PARAMETER_SCOPE::NONE;
  ParameterRef parameters[4] = {};
};

class SIDSynthEditor : public Menu {
public:
  SIDSynthEditor() : Menu("SID SYNTH") {}
  DELETE_COPY_MOVE(SIDSynthEditor);

  void HandleMenuEvent(MENU_EVENT menu_event) final;
  void HandleEvent(const Event &event) final;
  void UpdateDisplay() const final;
  void Step() final {}

  void register_listener(ParameterListener *listener) { listeners_.push_back(listener); }

private:
  int menu_level_ = -1;
  int menu_subpage_ = 0;
  EDITOR_PAGE editor_page_ = EDITOR_PAGE::NONE;
  std::array<ParameterValue *, 4> values_ = {};

  mutable char name_buf_[21] = {};
  mutable char value_buf_[21] = {};

  sidbits::VOICE_INDEX voice_index_ = sidbits::VOICE1;
  LFO_INDEX lfo_index_ = LFO1;

  char navigation_[3] = "";

  util::StaticStack<ParameterListener *, 8> listeners_;

  void CycleVoiceOrLfo();
  void SetMenuLevel(int level, bool force = false);
  void SetPage(EDITOR_PAGE editor_page, bool force);
};

}  // namespace pfm2sid::synth

ENABLE_ENUM_TO_INDEX(pfm2sid::synth::EDITOR_PAGE);

#endif  // PFM2SID_MENU_SYNTH_EDITOR_H_
