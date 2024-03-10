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
#include "display.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "stm32x/stm32x_core.h"

namespace pfm2sid {

void Display::Clear()
{
  // lcd.Clear();
  memset(line_buffer_, ' ', sizeof(line_buffer_));
  memset(status_bar_, ' ', kNumStatusIcons);
}

static char fmt_buf[Display::kLineWidth + 1] INCCMZ;

// We don't actually want the \0 in the screen buffer, but do want the convenience of printf.
// So this ends up being mildly awkward and not exactly efficient...
void Display::Fmt(uint8_t line, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  auto n = vsnprintf(fmt_buf, sizeof(fmt_buf), fmt, args);
  if (n < 0) n = 0;  // ignore encoding errors
  auto *buf = line_buffer_ + (line * kLineWidth);
  if (n < kLineWidth) {
    memcpy(buf, fmt_buf, n);
    memset(buf + n, ' ', kLineWidth - n);
  } else {
    memcpy(buf, fmt_buf, kLineWidth);
  }
  va_end(args);
}

void Display::Write(uint8_t line, const char *data)
{
  memcpy(line_buffer_ + (line * kLineWidth), data, kLineWidth);
}

void Display::Clear(uint8_t line)
{
  memset(line_buffer_ + (line * kLineWidth), ' ', kLineWidth);
}

void Display::Update()
{
  int i = 0;
  for (auto &icon : status_icons_) {
    if (icon.ticks > 1) {
      --icon.ticks;
      status_bar_[i] = icon.character;
    } else if (icon.ticks == 1) {
      status_bar_[i] = ' ';
    }
    ++i;
  }
  if (enable_status_bar_) {
    memcpy(line_buffer_ + (kLineWidth - kNumStatusIcons), status_bar_, kNumStatusIcons);
  }
}

static constexpr uint8_t kCharsPerTick = 4;  // ca 100us per char?

void Display::Tick()
{
  auto row = scan_row_;
  auto col = scan_col_;

  if (col < Lcd::kCols) {
    // Send data, up to end of current line
    auto nchars = std::min<uint8_t>(Lcd::kCols - col, kCharsPerTick);
    lcd.Print(scan_pos_, nchars);
    memcpy(lcd_pos_, scan_pos_, nchars);  // TODO There shouldn't be a race condition here?
    scan_pos_ += nchars;
    lcd_pos_ += nchars;
    col += nchars;
  } else {
    // Advance to next line
    row = (row + 1) % Lcd::kRows;
    col = 0;
    scan_pos_ = line_buffer_ + (row * kLineWidth);
    lcd_pos_ = lcd_buffer_ + (row * kLineWidth);

    // Check if there's a change in this line. This effectively skips it and falls through to the
    // next line in the next Tick. Only update the cursor if we actually find a changed character.
    for (; col < kLineWidth; ++col)
      if (lcd_pos_[col] != scan_pos_[col]) {
        lcd.MoveCursor(row, col);
        break;
      }
    scan_pos_ += col;
    lcd_pos_ += col;
  }

  scan_row_ = row;
  scan_col_ = col;
}

void Display::EnableStatusBar(bool enable)
{
  if (enable_status_bar_ != enable) { enable_status_bar_ = enable; }
}

}  // namespace pfm2sid
