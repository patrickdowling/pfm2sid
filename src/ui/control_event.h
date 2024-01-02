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
#ifndef PFM2SID_UI_CONTROL_EVENT_H_
#define PFM2SID_UI_CONTROL_EVENT_H_

#include <cstdint>

namespace pfm2sid {

enum class CONTROL : int {
  SWITCH1,
  SWITCH2,
  SWITCH3,
  SWITCH4,
  SWITCH5,
  SWITCH6,
  SWITCH7,
  SWITCH8,
  ENCODER1,
  ENCODER2,
  ENCODER3,
  ENCODER4,
  LAST
};

enum class TIMER { INVALID, BOOTSCREEN };

enum EVENT_TYPE {
  EVENT_NONE,
  EVENT_BUTTON_PRESS,
  EVENT_BUTTON_LONG_PRESS,
  EVENT_BUTTON_RELEASE,
  EVENT_ENCODER,
  EVENT_TIMER
};

struct Event {
  EVENT_TYPE type;
  union {  // is this even?
    CONTROL control;
    TIMER timer;
  };
  int32_t value = 0;
  uint32_t ticks = 0;

  constexpr Event() : type(EVENT_NONE), control(CONTROL::LAST) {}
  constexpr Event(EVENT_TYPE t, CONTROL c, int32_t v) : type(t), control(c), value(v) {}
  constexpr Event(EVENT_TYPE t, CONTROL c, int32_t v, uint32_t tk)
      : type(t), control(c), value(v), ticks(tk)
  {}
  constexpr Event(TIMER t, int32_t v) : type(EVENT_TIMER), timer(t), value(v) {}
};

constexpr int encoder_index(CONTROL control)
{
  return static_cast<int>(control) - static_cast<int>(CONTROL::ENCODER1);
}

constexpr int switch_index(CONTROL control)
{
  return static_cast<int>(control) - static_cast<int>(CONTROL::SWITCH1);
}

}  // namespace pfm2sid

#endif  // PFM2SID_UI_CONTROL_EVENT_H_
