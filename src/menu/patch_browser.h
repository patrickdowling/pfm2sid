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
#ifndef PFM2SID_MENU_PATCHES_H_
#define PFM2SID_MENU_PATCHES_H_

#include "misc/cursor.h"
#include "synth/synth.h"
#include "ui/menu.h"
#include "util/util_macros.h"

namespace pfm2sid::synth {

class PatchBrowser : public Menu {
public:
  PatchBrowser() : Menu("PATCHES", false) {}
  DELETE_COPY_MOVE(PatchBrowser);

  void MenuInit() final;
  void HandleEvent(const Event &event) final;
  void UpdateDisplay() const final;
  void Step() final;

protected:
  void MenuEnter() final;
  void Exit();

  void Load();
  void Save();

  util::Cursor<4> cursor_{0, kNumPatchesPerBank};
  bool saving_ = false;
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_MENU_PATCHES_H_
