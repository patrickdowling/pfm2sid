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
#ifndef PFM2SID_DEBUG_H_
#define PFM2SID_DEBUG_H_

#include "drivers/pfm2sid_gpio.h"
#include "stm32x/stm32x_debug.h"
#include "util/util_macros.h"
#include "util/util_pin_trace.h"

namespace pfm2sid {

namespace DEBUG {
void Init();

// NOTE: These collide with AIN1-4 so beware if we ever use them...
using DEBUG_PIN_1 = GPIO::AIN1;
using DEBUG_PIN_2 = GPIO::AIN2;
using DEBUG_PIN_3 = GPIO::AIN3;
using DEBUG_PIN_4 = GPIO::AIN4;
};  // namespace DEBUG

}  // namespace pfm2sid

#ifdef PFM2SID_DEBUG_ENABLE
#define PFM2SID_DEBUG_INIT() pfm2sid::DEBUG::Init()

#define PFM2SID_DEBUG_TRACE(pin) \
  stm32x::ScopedPinTrace<CONCAT(pfm2sid::DEBUG::DEBUG_PIN_, pin)> CONCAT(trace_pin_, pin){};

#else
#define PFM2SID_DEBUG_INIT() \
  do {                       \
  } while ()

#define PFM2SID_DEBUG_TRACE(pin) \
  do {                           \
  } while ()

#endif

#endif  // PFM2SID_DEBUG_H_
