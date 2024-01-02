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
#ifndef PFM2SID_MENU_SID_PLAYER_H_
#define PFM2SID_MENU_SID_PLAYER_H_

#include <cinttypes>

#include "menu/menu_util.h"
#include "sidbits/sid_stream.h"
#include "sidbits/sidbits.h"
#include "synth/engine.h"
#include "ui/display.h"
#include "ui/menu.h"

namespace pfm2sid {

extern synth::Engine engine;

// This isn't super useful (yet) since it can only play an in-memory stream.
// We could upload things via SYSEX but it'll make more sense once there's a file system.
class SIDPlayer : public Menu {
public:
  SIDPlayer() : Menu("\001 SID PLAYER \001") {}
  DELETE_COPY_MOVE(SIDPlayer);

  void Init(const sidbits::SIDStreamData &stream_data) { sid_stream_.Init(stream_data); }

  void Step() { sid_stream_.Step(); }

  void HandleMenuEvent(MENU_EVENT menu_event) final
  {
    switch (menu_event) {
      case MENU_EVENT::ENTER: engine.Reset(); break;
      case MENU_EVENT::EXIT: break;
    }
  }

  void HandleEvent(const Event &event) final
  {
    switch (event.type) {
      case EVENT_ENCODER: break;
      case EVENT_BUTTON_PRESS:
        switch (event.control) {
          case CONTROL::SWITCH1:
            engine.Reset();
            sid_stream_.Reset();
            break;
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
      menu::HexdumpRegisters(sid_stream_.register_map());
    } else {
      display.Fmt(0, "%14s", name());

      static const char progress[] = "############            ";
      static_assert(sizeof(progress) == 2 * 12 + 1);

      auto pct = sid_stream_.percent();
      display.Fmt(2, "[%.*s]%5.1f%%", 12, progress + 12 - (int)(pct / 100.f * 12.f), PRINT_F32(pct));
      display.Fmt(3, "%20" PRIu32, stats::render_block_cycles.value_in_us());
    }
  }

  auto register_map() const { return sid_stream_.register_map(); }

private:
  sidbits::SIDStream sid_stream_;

  bool hexdump_ = false;
};

}  // namespace pfm2sid

#endif  // PFM2SID_MENU_SID_PLAYER_H_
