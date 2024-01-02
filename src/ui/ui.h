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
#ifndef PFM2SID_UI_H_
#define PFM2SID_UI_H_

#include <array>

#include "control_event.h"
#include "drivers/encoder.h"
#include "drivers/shift_register.h"
#include "misc/simple_timer.h"
#include "stm32x/stm32x_switchstate.h"
#include "util/util_ringbuffer.h"

namespace pfm2sid {

class Menu;

class Ui {
public:
  Ui() = default;
  DELETE_COPY_MOVE(Ui);

  static constexpr size_t kNumTimerSlots = 4;
  static constexpr size_t kMaxEventQueueSize = 16;

  static constexpr uint32_t kLongPressTicks = 1000;  // tick = ms

  void Init();
  void Tick();
  void DispatchEvents();
  void UpdateDisplay() const;

  void AddTimer(TIMER timer_id, uint32_t timeout);

  void SetMenu(Menu *menu);

  void Step();

private:
  static constexpr size_t kNumSwitches = 8;
  static constexpr size_t kNumEncoders = 4;

  uint32_t ticks_ = 0;

  class SwitchStateEx : public stm32x::SwitchState {
  public:
    uint32_t press_time = 0;
    bool ignore_press = false;
  };

  std::array<SwitchStateEx, kNumSwitches> switch_states_;

  std::array<Encoder, kNumEncoders> encoders_;

  util::RingBuffer<Event, kMaxEventQueueSize> events_;

  std::array<util::SimpleTimer<TIMER, TIMER::INVALID>, kNumTimerSlots> timer_slots_;

  Menu *current_menu_ = nullptr;
};

extern Ui ui;

}  // namespace pfm2sid

#endif  // PFM2SID_UI_H_
