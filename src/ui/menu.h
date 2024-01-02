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
#ifndef PFM2SID_UI_MENU_H_
#define PFM2SID_UI_MENU_H_

#include "control_event.h"

namespace pfm2sid {

enum class MENU_EVENT { ENTER, EXIT };

class Menu {
public:
  virtual ~Menu() = default;

  const char *name() const { return name_; }

  virtual void HandleMenuEvent(MENU_EVENT) = 0;
  virtual void HandleEvent(const Event &) = 0;
  virtual void UpdateDisplay() const = 0;

  virtual void Step() = 0;

protected:
  Menu(const char *name) : name_(name) {}

  const char *const name_ = nullptr;
};

}  // namespace pfm2sid

#endif  // PFM2SID_UI_MENU_H_
