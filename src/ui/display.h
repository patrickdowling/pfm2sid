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
#ifndef PFM2SID_DISPLAY_H_
#define PFM2SID_DISPLAY_H_

#include <array>
#include <cstdint>

#include "drivers/lcd.h"
#include "status_icons.h"

namespace pfm2sid {

// This is a framebuffer type layer on top of the raw LCD commands.
//
// The idea is that we can only update the parts of the screen that need it (assuming callers
// exercise some restraint). Since updates are "slow" (even more so because of the 4-bit driver) we
// can also break the updates into smaller chunks.
//
// Initial measurements show the status bar update (move cursor + 4 icons) clocks in at 500us and
// full lines at > 1ms. Which is about the right order of magnitude given the 45us delay per nibble
// in the Lcd driver, but this is a long time to wait during normal processing. So the ::Tick
// function here sends some small amount.
//
// Even if we send 1 character every Tick we still have an update rate of 1000/(20x4) = 12.5 fps
// which may be visible on full-screen transitions.
// The "dirty" check is fairly simple and finds the first changed character in a line.
//
// Initially the status bar was prioritized and updated more frequently, but this doesn't actually
// seem necessary.
//
class Display {
public:
  static constexpr int kLineWidth = Lcd::kCols;
  static constexpr int kNumLines = Lcd::kRows;

  static constexpr int kNumStatusIcons = util::enum_count<ICON_POS>();

  void Init();

  void Clear();

  void Update();  // Regular update function to drive status icons
  void Tick();    // Scan/write to LCD

  void Fmt(uint8_t line, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
  void Write(uint8_t line, const char *data);
  void Clear(uint8_t line);

  template <ICON_POS icon_pos>
  void SetIcon(char c, int ttl)  // ttl should be > 1
  {
    status_icons_[util::enum_to_i(icon_pos)] = {c, ttl};
  }

  void EnableStatusBar(bool enable);

private:
  char line_buffer_[kNumLines * kLineWidth] = {};  // buffer application writes to
  char lcd_buffer_[kNumLines * kLineWidth] = {};   // cache of what's on LCD

  uint_fast8_t scan_row_ = 0;
  uint_fast8_t scan_col_ = 0;
  const char *scan_pos_ = line_buffer_;
  char *lcd_pos_ = lcd_buffer_;

  struct StatusIcon {
    char character = 0;
    int ticks = 0;
  };

  std::array<StatusIcon, kNumStatusIcons> status_icons_;
  char status_bar_[kNumStatusIcons + 1] = {0};
  bool enable_status_bar_ = true;
};

extern Display display;

}  // namespace pfm2sid

#endif  // PFM2SID_DISPLAY_H_
