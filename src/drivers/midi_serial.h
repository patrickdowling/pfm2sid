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
#ifndef PFM2SID_MIDI_SERIAL_H_
#define PFM2SID_MIDI_SERIAL_H_

#include <cstdint>
#include <optional>

#include "stm32x/stm32x_core.h"

namespace pfm2sid {

#define PFM2SID_MIDI_USART USART3

class MidiSerial {
public:
  MidiSerial() = default;
  DELETE_COPY_MOVE(MidiSerial);

  void Init();

  // We have multiple interrupts alrady so it seems find to poll the RX port instead of another
  // interrupt.
  // At 31250 Baud / (1 + 9 + 1) = 3125 bytes/s, 320us per byte so there's "plenty" of time, and the
  // processign will happen in the bottom half anyway.
  std::optional<uint8_t> Receive() const
  {
    if (PFM2SID_MIDI_USART->SR & USART_FLAG_RXNE) {
      return PFM2SID_MIDI_USART->DR;
    } else {
      return std::nullopt;
    }
  }
};

}  // namespace pfm2sid

#endif  // PFM2SID_MIDI_SERIAL_H_
