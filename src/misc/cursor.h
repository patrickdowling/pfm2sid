// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2024 Patrick Dowling (pld@gurkenkiste.com)
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
#ifndef PFM2SID_UTIL_CURSOR_H_
#define PFM2SID_UTIL_CURSOR_H_

#include <algorithm>

namespace util {

// "Fancy" cursor helper that tries to maintain the screen line away from the the top/bottom to
// indicate when the cursor is at the beginning/end. I borrowed this from o_C and it still seems
// super convoluted.

template <int num_lines>
class Cursor {
public:
  Cursor(int start, int end) : start_(start), end_(end) {}

  int pos() const { return pos_; }

  void Scroll(int amount)
  {
    auto pos = std::clamp(pos_ + amount, start_, end_ - 1);
    auto line = line_ + amount;

    if (amount < 0) {
      if (line < 2) {
        if (pos > start_)
          line = 1;
        else
          line = 0;
      }
    } else {
      if (line >= (num_lines - 2)) {
        if (pos < end_ - 1)
          line = num_lines - 2;
        else
          line = num_lines - 1;
      }
    }

    pos_ = pos;
    line_ = line;
  }

  auto first() const { return pos_ - line_; }
  auto last() const { return pos_ - line_ + (num_lines - 1); }

private:
  int start_ = 0;
  int end_ = 0;
  int line_ = 0;
  int pos_ = 0;
};

}  // namespace util

#endif  // PFM2SID_UTIL_CURSOR_H_
