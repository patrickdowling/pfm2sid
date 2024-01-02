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
#ifndef PFM2SID_CORE_TIMER_H_
#define PFM2SID_CORE_TIMER_H_

#include <cinttypes>

#include "stm32x/stm32x_core.h"
#include "stm32x/stm32x_debug.h"

namespace pfm2sid {

#define PFM2SID_CORE_TIMER_HANDLER TIM1_UP_TIM10_IRQHandler

// This handles both the system update timer/interrupt that drives the DAC, and adds an extra
// "precision" timer that's independent of SysTick (which might be running at 1KHz).
class CoreTimer {
public:
  void Init(uint32_t period);
  void Start();

  inline bool Ticked() const ALWAYS_INLINE
  {
    if (TIM1->SR & TIM_IT_Update) {
      TIM1->SR = (uint16_t)~TIM_IT_Update;
      return true;
    } else {
      return false;
    }
  }

  static inline uint32_t now() { return DWT->CYCCNT; }

  static constexpr uint32_t kCyclesPerMicro = F_CPU / 1'000'000UL;

  static constexpr uint32_t us_to_timer(uint32_t us) { return us * kCyclesPerMicro; }
  static constexpr uint32_t ms_to_timer(uint32_t ms) { return ms * 1000UL * kCyclesPerMicro; }

  static constexpr float timer_to_ms(uint32_t cycles)
  {
    return static_cast<float>(cycles) * (1000.f / static_cast<float>(F_CPU));
  }

  static inline void delay_us(uint32_t us)
  {
    auto start = now();
    while ((now() - start) < us_to_timer(us)) {}
  }
};

extern CoreTimer core_timer;

}  // namespace pfm2sid

#endif  // PFM2SID_CORE_TIMER_H_
