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
#ifndef PFM2SID_DAC_4922_H_
#define PFM2SID_DAC_4922_H_

#include <cinttypes>

#include "pfm2sid_gpio.h"
#include "stm32x/stm32x_gpio_utils.h"

namespace pfm2sid {

class MCP4922 {
public:
  enum uint16_t {
    SELECT_A = 0x0000,
    SELECT_B = 0x8000,
    BUF_ENABLE = 0x4000,
    BUF_DISABLE = 0x0000,
    GAIN_1X = 0x2000,
    GAIN_2X = 0x0000,
    ACTIVE = 0x1000,
  };
};

class Dac {
public:
  Dac() = default;
  DELETE_COPY_MOVE(Dac);

  enum DACS { DAC_R, DAC_L };
  enum STATE { BUSY, FRAME_COMPLETE };

  void Init();

  static constexpr int kResolution = 18;
  static constexpr int32_t kMaxValue = (1 << kResolution) - 1;
  static constexpr int32_t kMinValue = -(1 << kResolution);
  static constexpr int32_t kZeroValue = 1 << (kResolution - 1);

  static_assert(0x3ffff == kMaxValue);
  static_assert(0x20000 == kZeroValue);

  // For convenience these values are signed. They will be clipped and offset internally.
  void BeginFrame(int32_t l, int32_t r);

  [[nodiscard]] STATE Update()
  {
    auto state = state_;
    switch (state) {
      // Transfer<DAC_R>(values_[0]); => BeginFrame
      case 0: Transfer<DAC_L>(values_[1]); break;
      case 1: Transfer<DAC_R>(values_[2]); break;
      case 2: Transfer<DAC_L>(values_[3]); break;
      case 3: stm32x::Set<GPIO::DAC_L_CS, GPIO::DAC_R_CS>(); break;
    }

    ++state;
    state_ = state;
    return state > 3 ? FRAME_COMPLETE : BUSY;
  }

  bool frame_complete() const { return state_ > 3; }
  void Load() { GPIO::DAC_LDAC::Reset(); }

private:
  int state_ = 4;
  uint16_t values_[4] = {0};

  template <Dac::DACS dac>
  inline void Transfer(uint16_t value)
  {
    // NOTE
    // The datasheet says the setup time of 40ns between !CS and CLk only applies if CLK is high.
    // We're using SPI_CPOL_Low (gleaned from OG code) so that seems to not apply?
    static_assert(Dac::DAC_R == dac || Dac::DAC_L == dac);
    if constexpr (Dac::DAC_R == dac) {
      GPIO::DAC_R_CS::Reset();
      GPIO::DAC_L_CS::Set();
    } else {
      GPIO::DAC_L_CS::Reset();
      GPIO::DAC_R_CS::Set();
    }
    SPI1->DR = value;
  }
};

}  // namespace pfm2sid

#endif  // PFM2SID_DAC_4922_H_
