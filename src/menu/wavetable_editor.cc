// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2024 Patrick Dowling (pld@gurkenkiste.com)
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
#include "wavetable_editor.h"

#include "ui/display.h"
#include "ui/ui.h"

namespace pfm2sid {

namespace synth {

void WavetableEditor::MenuInit() {}

void WavetableEditor::MenuEnter() {}

void WavetableEditor::HandleEvent(const Event &event)
{
  if (EVENT_ENCODER == event.type) {
  } else if (EVENT_BUTTON_PRESS == event.type) {
    switch (event.control) {
      case CONTROL::SWITCH6: break;
      case CONTROL::SWITCH7: Exit(); break;
      default: break;
    }
  } else if (EVENT_BUTTON_LONG_PRESS == event.type) {
    switch (event.control) {
      default: break;
    }
  }
}

void WavetableEditor::UpdateDisplay() const {}

void WavetableEditor::Step() {}

void WavetableEditor::Exit()
{
  ui.SetMenu(back_);
}

}  // namespace synth

}  // namespace pfm2sid
