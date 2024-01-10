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
#ifndef PFM2SID_MENU_ASID_PLAYER_H_
#define PFM2SID_MENU_ASID_PLAYER_H_

#include <cinttypes>

#include "menu/menu_util.h"
#include "platform/platform.h"
#include "pfm2sid_stats.h"
#include "sidbits/asid_parser.h"
#include "sidbits/sidbits.h"
#include "synth/engine.h"
#include "ui/display.h"
#include "ui/menu.h"

namespace pfm2sid {

extern synth::Engine engine;

class ASIDPlayer : public Menu {
public:
  ASIDPlayer() : Menu("\001 ASID \001", true) {}
  DELETE_COPY_MOVE(ASIDPlayer);

  void Step() {}

  void MenuEnter() final { engine.Reset(); }

  void HandleEvent(const Event &event) final
  {
    switch (event.type) {
      case EVENT_ENCODER: break;
      case EVENT_BUTTON_PRESS:
        switch (event.control) {
          case CONTROL::SWITCH1:
            engine.Reset();
            asid_parser_.Reset();
            break;
          case CONTROL::SWITCH6: set_mode(MODE::SID_SYNTH); break;
          case CONTROL::SWITCH7:
            hexdump_ = !hexdump_;
            display.Clear();
            break;
          default: break;
        }
        break;
      default: break;
    }
  }

  void UpdateDisplay() const final
  {
    if (hexdump_) {
      menu::HexdumpRegisters(asid_parser_.register_map());
    } else {
      display.Fmt(0, "%14s", name());
      display.Fmt(1, "%20s", asid_parser_.lcd_data());
      /*
      static const char progress[] = "############            ";
      static_assert(sizeof(progress) == 2 * 12 + 1);

      auto pct = sid_stream_.percent();
      display.Fmt(2, "[%.*s]%5.1f%%", 12, progress + 12 - (int)(pct / 100.f * 12.f), pct);
      */
      display.Fmt(2, "%20.2f", PRINT_F32(CoreTimer::timer_to_ms(delta_t_)));
      display.Fmt(3, "%20" PRIu32, stats::render_block_cycles.value_in_us());
    }
  }

  auto ParseSysex(const uint8_t *data, size_t len)
  {
    auto now = CoreTimer::now();
    delta_t_ = now - last_;
    last_ = now;

    return asid_parser_.ParseSysex(data, len);
  }

  const auto &register_map() const { return asid_parser_.register_map(); }

private:
  sidbits::ASIDParser asid_parser_;

  uint32_t last_ = 0;
  uint32_t delta_t_ = 0;

  bool hexdump_ = false;
};

}  // namespace pfm2sid

#endif  // PFM2SID_MENU_ASID_PLAYER_H_
