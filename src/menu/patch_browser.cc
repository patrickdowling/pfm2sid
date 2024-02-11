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
#include "patch_browser.h"

#include "synth/patch_bank.h"
#include "ui/display.h"
#include "ui/ui.h"

namespace pfm2sid {

extern synth::PatchBank current_bank;
extern synth::Patch current_patch;

namespace synth {

void PatchBrowser::MenuInit() {}

void PatchBrowser::MenuEnter()
{
  saving_ = false;
}

void PatchBrowser::HandleEvent(const Event &event)
{
  if (EVENT_ENCODER == event.type) {
    cursor_.Scroll(event.value);
  } else if (EVENT_BUTTON_PRESS == event.type) {
    switch (event.control) {
      case CONTROL::SWITCH6:
        if (saving_)
          Save();
        else
          Load();
        Exit();
        break;
      case CONTROL::SWITCH7: Exit(); break;
      default: break;
    }
  } else if (EVENT_BUTTON_LONG_PRESS == event.type) {
    switch (event.control) {
      case CONTROL::SWITCH7: saving_ = !saving_; break;
      default: break;
    }
  }
}

void PatchBrowser::UpdateDisplay() const
{
  int l = 0;
  for (int i = cursor_.first(); i <= cursor_.last(); ++i) {
    char action = ' ';
    if (i == cursor_.pos()) action = saving_ ? ICON_PATCH_SAVE : ICON_PATCH_LOAD;
    char current = i == current_patch.number() ? '*' : ' ';
    display.Fmt(l++, "%c%02d%c%-15s", action, i, current, current_bank.patch_name(i));
  }
}

void PatchBrowser::Step() {}

void PatchBrowser::Exit()
{
  ui.SetMenu(back_);
}

void PatchBrowser::Load()
{
  current_bank.Load(cursor_.pos(), current_patch);
  current_patch.set_number(cursor_.pos());
}

void PatchBrowser::Save()
{
  current_bank.Save(cursor_.pos(), current_patch);
  current_patch.set_number(cursor_.pos());
}

}  // namespace synth

}  // namespace pfm2sid
