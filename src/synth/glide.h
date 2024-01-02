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
#ifndef PFM2SID_SYNTH_GLIDE_H_
#define PFM2SID_SYNTH_GLIDE_H_

#include <cstdint>

#include "midi/midi_types.h"
#include "misc/fixed_point.h"

namespace pfm2sid::synth {

// There are two implementation options
// 1/ calculate the increment based on target - value ahead of time
// 2/ use a phase counter
// There doesn't seem to be a clear winner.
//
class Glide {
public:
  // TODO Depending on the range of times we may need to use a .24 here
  // \sa tests that check for duplicate values in LUT
  using value_type = util::s816;

  void Reset()
  {
    value_ = target_value_ = 0;
    increment_ = 0;
  }

  void Init(midi::Note note, int32_t rate)
  {
    target_value_ = note;
    auto delta = target_value_ - value_;

    if (!rate || !delta) {
      value_ = target_value_;
      increment_ = 0;
    } else {
      auto inc = delta.scale(glide_increment(rate));
      if (!inc) inc = delta.value() > 0 ? 1 : -1;
      increment_ = inc;
    }
  }

  auto Update() -> value_type
  {
    if (!increment_) {
      return value_;
    } else {
      auto value = value_;
      value += increment_;

      // TODO There must be a more elegant way...
      if ((increment_ > 0 && value >= target_value_) ||
          (increment_ < 0 && value <= target_value_)) {
        increment_ = 0;
        value = target_value_;
      }
      value_ = value;
      return value;
    }
  }

  bool active() const { return increment_ != 0; }

private:
  value_type value_;
  value_type target_value_;

  int32_t increment_ = 0;

  // Testable
public:
  static int32_t glide_increment(int32_t rate);
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_GLIDE_H_
