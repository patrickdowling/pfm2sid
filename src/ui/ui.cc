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
#include "ui.h"

#include <cstdio>

#include "display.h"
#include "menu.h"
#include "pfm2sid_stats.h"
#include "resources.h"

namespace pfm2sid {
Display display INCCM;
Lcd lcd INCCM;
ShiftRegister shift_register INCCM;

void Ui::Init()
{
  GPIO::STATUS_LED::Init();
  lcd.Init();
  shift_register.Init();

  for (int i = 0; i < resources::LCD_CHAR_LAST; ++i) {
    lcd.DefineUserChar(to_char(static_cast<resources::LCD_CHAR_RESID>(i)), resources::lcd_chars[i]);
  }
  lcd.Clear();
  display.Clear();
}

#define BITMASK(bit) (1 << bit)

static constexpr struct {
  CONTROL control;
  ShiftRegister::value_type bitmask;
  bool long_press = false;
} kSwitchDescs[] = {
    {CONTROL::SWITCH1, BITMASK(8)},       {CONTROL::SWITCH2, BITMASK(15)},
    {CONTROL::SWITCH3, BITMASK(9)},       {CONTROL::SWITCH4, BITMASK(14)},
    {CONTROL::SWITCH5, BITMASK(10)},      {CONTROL::SWITCH6, BITMASK(4), true},
    {CONTROL::SWITCH7, BITMASK(3), true}, {CONTROL::SWITCH8, BITMASK(5)},
};

static constexpr struct {
  CONTROL control;
  ShiftRegister::value_type bitmask[2];
} kEncoderPins[] = {
    {CONTROL::ENCODER1, {BITMASK(11), BITMASK(13)}},
    {CONTROL::ENCODER2, {BITMASK(0), BITMASK(12)}},
    {CONTROL::ENCODER3, {BITMASK(1), BITMASK(7)}},
    {CONTROL::ENCODER4, {BITMASK(2), BITMASK(6)}},
};

void Ui::Tick()
{
  const auto register_value = shift_register.Read();
  auto now = ticks_;

  size_t idx = 0;
  for (auto &switch_state : switch_states_) {
    const auto &switch_desc = kSwitchDescs[idx];
    ++idx;

    switch_state.Poll(register_value, switch_desc.bitmask);

    // This is all bit convoluted to try and handle long press vs. normal press.
    // Especially the just_released() path seems somewhat inconsistent.

    if (switch_state.just_pressed()) {
      switch_state.press_time = ticks_;
      if (!switch_desc.long_press) events_.EmplaceWrite(EVENT_BUTTON_PRESS, switch_desc.control, 0);
    } else if (switch_state.just_released()) {
      if (!switch_desc.long_press)
        events_.EmplaceWrite(EVENT_BUTTON_RELEASE, switch_desc.control, 0,
                             now - switch_state.press_time);
      else if (!switch_state.ignore_press)
        events_.EmplaceWrite(EVENT_BUTTON_PRESS, switch_desc.control, 0,
                             now - switch_state.press_time);

      switch_state.press_time = 0;
      switch_state.ignore_press = false;
    } else if (switch_state.pressed() && switch_desc.long_press) {
      if (!switch_state.ignore_press && switch_state.press_time &&
          (now - switch_state.press_time) > kLongPressTicks) {
        events_.EmplaceWrite(EVENT_BUTTON_LONG_PRESS, switch_desc.control, 0,
                             now - switch_state.press_time);
        switch_state.ignore_press = true;
      }
    }
  }

  idx = 0;
  for (auto &enc : encoders_) {
    auto value =
        enc.Poll(register_value, kEncoderPins[idx].bitmask[0], kEncoderPins[idx].bitmask[1]);
    if (value) events_.EmplaceWrite(EVENT_ENCODER, kEncoderPins[idx].control, value);
    ++idx;
  }

  ticks_ = now + 1;
}

void Ui::DispatchEvents()
{
  // This might be better off in Tick() but that's a different "thread". These are low precision
  // and rare events anyway (famous last worsds).
  auto now = STM32X_CORE_NOW();
  for (auto &timer : timer_slots_) {
    if (timer.enabled()) {
      timer.Update(now);
      if (timer.elapsed()) {
        timer.Reset();
        if (current_menu_) current_menu_->HandleEvent({timer.id(), 0});
      }
    }
  }

  while (events_.readable()) {
    const auto event = events_.Read();
    ++stats::ui_event_counter;
    if (current_menu_) current_menu_->HandleEvent(event);
  }
}

void Ui::UpdateDisplay() const
{
  if (current_menu_) current_menu_->UpdateDisplay();
  display.Update();
}

void Ui::AddTimer(TIMER timer_id, uint32_t timeout)
{
  for (auto &timer : timer_slots_) {
    if (!timer.enabled()) {
      timer.Arm(timer_id, STM32X_CORE_NOW(), timeout);
      break;
    }
  }
}

void Ui::SetMenu(Menu *menu)
{
  if (menu != current_menu_) {
    if (current_menu_) current_menu_->HandleMenuEvent(MENU_EVENT::EXIT);
    if (menu) {
      //display.Fmt(0, "%-16s", menu->name());
      menu->HandleMenuEvent(MENU_EVENT::ENTER);
      menu->set_back(current_menu_);
    }
    current_menu_ = menu;
  }
}

void Ui::Step()
{
  if (current_menu_) current_menu_->Step();
}

}  // namespace pfm2sid
