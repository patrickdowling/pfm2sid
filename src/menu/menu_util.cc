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
#include "menu_util.h"

#include "ui/display.h"

namespace pfm2sid::menu {

void HexdumpRegisters(const sidbits::RegisterMap &register_map)
{
  // NOTE This prints bottom to top to preserve the status bar

  uint8_t line = 3;
  uint8_t offset = 0;
  for (uint8_t v = 0; v < 3; ++v) {
    display.Fmt(line--, "%02X %02X%02X %02X%02X %02X %02X%02X", offset,
                register_map.peek(offset + 0), register_map.peek(offset + 1),
                register_map.peek(offset + 2), register_map.peek(offset + 3),
                register_map.peek(offset + 4), register_map.peek(offset + 5),
                register_map.peek(offset + 6));
    offset += 7;
  }
  display.Fmt(line, "%02X %02X%02X %02X%02X", offset, register_map.peek(offset + 0),
              register_map.peek(offset + 1), register_map.peek(offset + 2),
              register_map.peek(offset + 3));
}

}  // namespace pfm2sid::menu
