// pfm2sid
// Copyright (C) 2023 Patrick Dowling (pld@gurkenkiste.com)
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
#ifndef PFM2SID_ENCODER_H_
#define PFM2SID_ENCODER_H_

#include <cstdint>

#include "util/util_macros.h"

namespace pfm2sid {

// For a change this uses a state table to figure out the transitions.

class Encoder {
public:
  Encoder() = default;
  DELETE_COPY_MOVE(Encoder);

  static constexpr int kIncrement = 2;

  void reverse(bool r) { reverse_ = r; }

  template <typename T>
  int32_t Poll(const T bits, const T pina_bitmask, const T pinb_bitmask)
  {
    unsigned state = (state_ << 2) & 0x0f;
    if (bits & pina_bitmask) state |= 0x2;
    if (bits & pinb_bitmask) state |= 0x1;
    auto t = kStateTable[state];
    state_ = state;

    int inc = 0;
    if (t) {
      auto steps = steps_ + t;
      if (steps >= kIncrement) {
        inc = steps / kIncrement;
        steps -= kIncrement;
      } else if (steps <= -kIncrement) {
        inc = steps / kIncrement;
        steps += kIncrement;
      }
      steps_ = steps;
    }
    return reverse_ ? inc : -inc;
  }

private:
  unsigned state_ = 0U;
  int steps_ = 0;
  bool reverse_ = false;

  static constexpr int kStateTable[16] = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
};

}  // namespace pfm2sid

#endif  // PFM2SID_ENCODER_H_
