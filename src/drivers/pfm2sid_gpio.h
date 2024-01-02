// Copyright 2023 Patrick Dowling
//
// Author: Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// AUTOMATICALLY GENERATED FILE, DO NOT EDIT
// clang-format off
// IOC source: ./stm32/preenF405.ioc
// CSV source: ./stm32/preenF405.csv
// --------------------------------------------------------------------------------
#ifndef PFM2SID_GPIO_H_
#define PFM2SID_GPIO_H_
#include "stm32x/stm32x_core.h"

namespace pfm2sid {
class GPIO {
public:
  GPIO() = default;
  DELETE_COPY_MOVE(GPIO);

  using LCD_RS = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 0, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using LCD_E = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 1, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using LCD_D0 = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 2, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using LCD_D1 = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 3, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using LCD_D2 = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 4, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using LCD_D3 = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 5, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using HC165_SHLD = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 6, stm32x::GPIO_SPEED::MEDIUM, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using HC165_DATA = stm32x::GPIO_IN<stm32x::GPIO_PORT_A, 7, stm32x::GPIO_PUPD::NONE, false>;
  using HC165_CLK = stm32x::GPIO_OUT<stm32x::GPIO_PORT_A, 8, stm32x::GPIO_SPEED::MEDIUM, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using USB_OTG_FS_DM = stm32x::GPIO_AF<stm32x::GPIO_PORT_A, 11, stm32x::GPIO_SPEED::MEDIUM, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, 1/*GPIO_AF10_OTG_FS*/, false>;
  using USB_OTG_FS_DP = stm32x::GPIO_AF<stm32x::GPIO_PORT_A, 12, stm32x::GPIO_SPEED::MEDIUM, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, 1/*GPIO_AF10_OTG_FS*/, false>;
  using SPI1_SCK = stm32x::GPIO_AF<stm32x::GPIO_PORT_B, 3, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, 5/*GPIO_AF5_SPI1*/, false>;
  using DAC_R_CS = stm32x::GPIO_OUT<stm32x::GPIO_PORT_B, 4, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using SPI1_MOSI = stm32x::GPIO_AF<stm32x::GPIO_PORT_B, 5, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, 5/*GPIO_AF5_SPI1*/, false>;
  using STATUS_LED = stm32x::GPIO_OUT<stm32x::GPIO_PORT_B, 6, stm32x::GPIO_SPEED::MEDIUM, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using DAC_L_CS = stm32x::GPIO_OUT<stm32x::GPIO_PORT_B, 9, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;
  using MIDI_TX = stm32x::GPIO_AF<stm32x::GPIO_PORT_B, 10, stm32x::GPIO_SPEED::LOW, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::PULLUP, 7/*GPIO_AF7_USART3*/, false>;
  using MIDI_RX = stm32x::GPIO_AF<stm32x::GPIO_PORT_B, 11, stm32x::GPIO_SPEED::LOW, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, 7/*GPIO_AF7_USART3*/, false>;
  using USB_OTG_HS_DM = stm32x::GPIO_AF<stm32x::GPIO_PORT_B, 14, stm32x::GPIO_SPEED::MEDIUM, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, 1/*GPIO_AF12_OTG_HS_FS*/, false>;
  using USB_OTG_HS_DP = stm32x::GPIO_AF<stm32x::GPIO_PORT_B, 15, stm32x::GPIO_SPEED::MEDIUM, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, 1/*GPIO_AF12_OTG_HS_FS*/, false>;
  using AIN1 = stm32x::GPIO_AN<stm32x::GPIO_PORT_C, 0, false>;
  using AIN2 = stm32x::GPIO_AN<stm32x::GPIO_PORT_C, 1, false>;
  using AIN3 = stm32x::GPIO_AN<stm32x::GPIO_PORT_C, 2, false>;
  using AIN4 = stm32x::GPIO_AN<stm32x::GPIO_PORT_C, 3, false>;
  using DAC_LDAC = stm32x::GPIO_OUT<stm32x::GPIO_PORT_C, 12, stm32x::GPIO_SPEED::FAST, stm32x::GPIO_OTYPE::PP, stm32x::GPIO_PUPD::NONE, false>;

  static void EnableClocks(bool enable = true) {
    stm32x::GPIOx<stm32x::GPIO_PORT_A>::EnableClock(enable);
    stm32x::GPIOx<stm32x::GPIO_PORT_B>::EnableClock(enable);
    stm32x::GPIOx<stm32x::GPIO_PORT_C>::EnableClock(enable);
  }
}; // class GPIO

extern GPIO gpio;
} // namespace pfm2sid
#endif // PFM2SID_GPIO_H_
