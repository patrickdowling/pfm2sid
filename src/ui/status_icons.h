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
#ifndef PFM2SID_UI_STATUS_ICONS_H_
#define PFM2SID_UI_STATUS_ICONS_H_

#include "resources.h"
#include "util/util_templates.h"

#define RIGHT_ARROW_STR "\x7e"
#define RIGHT_ARROW_CHAR '\x7e'
#define LEFT_ARROW_CHAR '\x7f'

namespace pfm2sid {

enum struct ICON_POS { VOICE1, VOICE2, VOICE3, MIDI, LAST };

// Ticks in this context are ca. 20ms

static constexpr int kMidiActivityTicks = 10;
static constexpr int kVoiceActivityTicks = 10;

template <resources::LCD_CHAR_RESID c>
constexpr char to_char()
{
  return 1 + c;
}

constexpr char to_char(resources::LCD_CHAR_RESID c)
{
  return 1 + c;
}

static constexpr char ICON_MIDI_NOTE = to_char<resources::LCD_CHAR_NOTE>();
static constexpr char ICON_MIDI_BEND = to_char<resources::LCD_CHAR_BEND>();
static constexpr char ICON_MIDI_SYSEX = '!';
static constexpr char ICON_VOICE_ACTIVE = to_char<resources::LCD_CHAR_VOICE_X1>();

static constexpr char ICON_PATCH_LOAD = LEFT_ARROW_CHAR;
static constexpr char ICON_PATCH_SAVE = RIGHT_ARROW_CHAR;

}  // namespace pfm2sid

ENABLE_ENUM_TO_INDEX(pfm2sid::ICON_POS);

#endif  // PFM2SID_UI_STATUS_ICONS_H_
