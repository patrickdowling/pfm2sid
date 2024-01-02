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
#include "shift_register.h"

#include "stm32x/stm32x_gpio_utils.h"

namespace pfm2sid {

#define NOP_DELAY(numnops)                                 \
  do {                                                     \
    for (int n = 0; n < numnops; ++n) asm volatile("nop"); \
  } while (0)

using CLK = GPIO::HC165_CLK;
using DATA = GPIO::HC165_DATA;
using SHLD = GPIO::HC165_SHLD;

void ShiftRegister::Init()
{
  SHLD::Init();
  CLK::Init();
  DATA::Init();

  CLK::Reset();
  SHLD::Reset();
}

// In this case, we have to bitbang the shift register since there's no hardware SPI on the pins.
// This is less than ideal since we have to "waste" some cycles since the GPIO are fast; the
// original code used the ST functions which are slow by nature.
//
// Yes, this is full of sketchy casts.
ShiftRegister::value_type ShiftRegister::Read() const
{
  SHLD::Reset();  // Load values
  NOP_DELAY(4);
  SHLD::Set();  // Shift enable
  NOP_DELAY(4);

  value_type value = 0;
  for (int i = 0; i < kNumBits; ++i) {
    value |= (DATA::Read() << i);
    CLK::Set();
    NOP_DELAY(4);
    CLK::Reset();
    NOP_DELAY(4);
  }

  return value;
}

}  // namespace pfm2sid
