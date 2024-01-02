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
#ifndef PFM2SID_SIDBITS_ASID_PARSER_H_
#define PFM2SID_SIDBITS_ASID_PARSER_H_

#include <cstdint>
#include <memory>
#include <optional>

#include "sidbits.h"

namespace pfm2sid::sidbits {

// Parse SID commands-over-MIDI using the ASID protocol, which sends register contents via sysex.
// Gleaned from a bunch of sources, including
// https://paulus.kapsi.fi/asid_protocol.txt

static constexpr uint8_t ASID_SYSEX_ID = 0x2D;
constexpr bool is_asid_sysex(const uint8_t *data)
{
  return data[0] == ASID_SYSEX_ID;
}

class ASIDParser {
public:
  static constexpr uint8_t START_SID = 0x4C;
  static constexpr uint8_t STOP_SID = 0x4D;
  static constexpr uint8_t COMMAND = 0x4E;
  static constexpr uint8_t LCD_DATA = 0x4F;

  static constexpr uint8_t ASID_REGISTER_COUNT = sidbits::SID_REGISTER_COUNT + 3;
  static constexpr size_t LCD_DATA_LEN = 20;

  void Reset();

  std::optional<size_t> ParseSysex(const uint8_t *data, size_t len);

  bool active() const { return active_; }

  const auto &register_map() const { return register_map_; }

  const char *lcd_data() const { return lcd_data_; }

private:
  bool active_ = false;

  char lcd_data_[LCD_DATA_LEN + 1] = {0};
  sidbits::RegisterMap register_map_;

  std::optional<size_t> ParseCommandMessage(const uint8_t *msg, size_t len);
  void ParseLcdDataMessage(const uint8_t *msg, size_t len);
};

}  // namespace pfm2sid::sidbits

#endif  // PFM2SID_SIDBITS_ASID_PARSER_H_
