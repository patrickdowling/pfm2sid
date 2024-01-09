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
#include "asid_parser.h"

#include <cstring>

namespace pfm2sid::sidbits {

void ASIDParser::Reset()
{
  register_map_.Reset();
  register_map_.filter_set_mode_volume(FILTER_MODE::OFF, 0x0f, false);
  active_ = false;
}

static constexpr uint8_t ASID_REGISTER_MAP[ASIDParser::ASID_REGISTER_COUNT] = {
    0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x04, 0x0b, 0x12, 0x04, 0x0b, 0x12};

std::optional<size_t> ASIDParser::ParseSysex(const uint8_t *data, size_t len)
{
  if (!data || len < 2 || !is_asid_sysex(data)) return std::nullopt;

  switch (data[1]) {
    case START_SID:
      strcpy(lcd_data_, "start");
      return 0;
      break;
    case STOP_SID:
      strcpy(lcd_data_, "stop");
      return 0;
      break;
    case COMMAND: return ParseCommandMessage(data + 2, len - 2); break;
    case LCD_DATA: return ParseLcdDataMessage(data + 2, len - 2); break;
  }

  return std::nullopt;
}

std::optional<size_t> ASIDParser::ParseCommandMessage(const uint8_t *msg, size_t len)
{
  // TODO Technically we can count the set bits and derive an expected length

  // [mask][mask][mask][mask][msb][msb][msb][msb](data)
  if (len < 8) return std::nullopt;

  if (!active()) { active_ = true; }

  // There are a few ways of implementing this. The key is to remember that all the bytes only
  // contain 7 bits so we get 4x7=28 registers.
  //
  // The first 8 bytes are meta information: Four bytes are a bit mask indicating
  // which registers are being set. There should be that many data bytes following the meta as there
  // are bits set. The next four are the MSB of the data bytes, since MIDI data is 7-bit.

  const uint8_t *mask_data = msg;

  const uint8_t *data = mask_data + 8;
  const uint8_t *end = msg + len;

  size_t register_count = 0;

  for (unsigned byte_index = 0; data < end && byte_index < 4; ++byte_index) {
    for (unsigned bit = 0; data < end && bit < 7; ++bit) {
      auto mask = 1 << bit;
      if (mask_data[byte_index] & mask) {
        auto asid_register = byte_index * 7 + bit;
        uint8_t data_byte = *data++;
        if (mask_data[4 + byte_index] & mask) data_byte |= 0x80;
        register_map_.poke(ASID_REGISTER_MAP[asid_register], data_byte);
        ++register_count;
      }
    }
  }

  return register_count;
}

size_t ASIDParser::ParseLcdDataMessage(const uint8_t *msg, size_t len)
{
  memset(lcd_data_, 0, sizeof(lcd_data_));
  auto dst = lcd_data_;
  len = std::min(len, LCD_DATA_LEN);

  std::transform(msg, msg + len, dst, [](uint8_t b) { return static_cast<char>(b); });
  dst[len] = 0;
  return len;
}

}  // namespace pfm2sid::sidbits
