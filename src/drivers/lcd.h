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
#ifndef PFM2SID_LCD_H_
#define PFM2SID_LCD_H_

#include <cinttypes>
#include <memory>

#include "util/util_macros.h"

namespace pfm2sid {

// This is probably going to be a mish-mash of buffered/unbuffered methods and assume a 4-bit data
// bus.
class Lcd {
public:
  // These are pretty much fixed
  static constexpr uint8_t kRows = 4;
  static constexpr uint8_t kCols = 20;

  Lcd() = default;
  DELETE_COPY_MOVE(Lcd);

  void Init();
  void Clear();
  void MoveCursor(uint8_t row, uint8_t col);
  void Print(const char *str);
  void Print(const char *std, size_t len);

  void DefineUserChar(uint8_t index, const uint8_t *data);
};

extern Lcd lcd;

}  // namespace pfm2sid

#endif  // PFM2SID_LCD_H_
