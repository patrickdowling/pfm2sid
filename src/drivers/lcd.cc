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
#include "lcd.h"

#include <algorithm>

#include "core_timer.h"
#include "pfm2sid_gpio.h"
#include "stm32x/stm32x_gpio_utils.h"

namespace pfm2sid {

enum LCD_CMD : uint8_t {
  LCD_CLEAR = 0x01,
  LCD_HOME = 0x02,
  LCD_ENTRY_MODE = 0x04,
  LCD_DISPLAY_STATUS = 0x08,
  LCD_CURSOR = 0x10,
  LCD_FUNCTION_SET = 0x20,
  LCD_SET_CGRAM_ADDRESS = 0x40,
  LCD_SET_DDRAM_ADDRESS = 0x80,

  LCD_SHIFT = 0x01,
  LCD_NO_SHIFT = 0x00,
  LCD_CURSOR_INCREMENT = 0x02,
  LCD_CURSOR_NO_INCREMENT = 0x00,
  LCD_DISPLAY_ON = 0x04,
  LCD_DISPLAY_OFF = 0x00,
  LCD_CURSOR_OFF = 0x00,
  LCD_CURSOR_ON = 0x02,
  LCD_BLINKING_ON = 0x01,
  LCD_BLINKING_OFF = 0x00,

  LCD_8BIT = 0x10,
  LCD_4BIT = 0x00,

  LCD_2_LINE = 0x08,
  LCD_1_LINE = 0x00,

  LCD_LARGE_FONT = 0x04,
  LCD_SMALL_FONT = 0x00,
};

static constexpr uint16_t kDelayAfterWrite = 45;

using LCD_RS = GPIO::LCD_RS;
using LCD_E = GPIO::LCD_E;
using LCD_DATA_PINS = stm32x::PinGroup<GPIO::LCD_D0, GPIO::LCD_D1, GPIO::LCD_D2, GPIO::LCD_D3>;

static void WriteNibbleImmediate(uint8_t value)
{
  LCD_DATA_PINS::Set(value);
  CoreTimer::delay_us(1);

  LCD_E::Set();
  CoreTimer::delay_us(1);
  LCD_E::Reset();
}

static void WriteCommandImmediate(uint8_t cmd, uint16_t delay_us = kDelayAfterWrite)
{
  LCD_RS::Reset();
  WriteNibbleImmediate(cmd >> 4);
  WriteNibbleImmediate(cmd);
  CoreTimer::delay_us(delay_us);
}

static void WriteDataImmediate(uint8_t data)
{
  LCD_RS::Set();
  WriteNibbleImmediate(data >> 4);
  WriteNibbleImmediate(data);
  CoreTimer::delay_us(kDelayAfterWrite);
}

void Lcd::Init()
{
  LCD_RS::Init();
  LCD_E::Init();
  LCD_DATA_PINS::Init();

  CoreTimer::delay_us(50 * 1000);

  LCD_RS::Reset();
  LCD_E::Reset();
  LCD_DATA_PINS::Set(0);

  for (int i = 0; i < 3; ++i) {
    WriteNibbleImmediate((LCD_FUNCTION_SET | LCD_8BIT) >> 4);
    CoreTimer::delay_us(2 * 1000);
  }
  WriteNibbleImmediate((LCD_FUNCTION_SET | LCD_4BIT) >> 4);
  CoreTimer::delay_us(2 * 1000);

  WriteCommandImmediate(LCD_FUNCTION_SET | LCD_4BIT | LCD_2_LINE | LCD_SMALL_FONT);
  WriteCommandImmediate(LCD_DISPLAY_STATUS | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINKING_OFF);
  WriteCommandImmediate(LCD_ENTRY_MODE | LCD_CURSOR_INCREMENT | LCD_NO_SHIFT);
  WriteCommandImmediate(LCD_CLEAR, 2 * 1000);
  WriteCommandImmediate(LCD_HOME, 2 * 1000);

  MoveCursor(0, 0);
}

void Lcd::Clear()
{
  WriteCommandImmediate(LCD_CLEAR, 1500);
  WriteCommandImmediate(LCD_HOME, 1500);
}

void Lcd::MoveCursor(uint8_t row, uint8_t col)
{
  static constexpr uint8_t kRowOffsets[] = {0x00, 0x40, 0x00 + kCols, 0x40 + kCols};

  row = std::clamp<uint8_t>(row, 0, kRows - 1);
  col = std::clamp<uint8_t>(col, 0, kCols - 1);
  WriteCommandImmediate(LCD_SET_DDRAM_ADDRESS | (col + kRowOffsets[row]));
}

void Lcd::Print(const char *str)
{
  while (*str) { WriteDataImmediate(*str++); }
}

void Lcd::Print(const char *str, size_t len)
{
  while (len--) { WriteDataImmediate(*str++); }
}

void Lcd::DefineUserChar(uint8_t index, const uint8_t *data)
{
  WriteCommandImmediate(LCD_SET_CGRAM_ADDRESS | (index * 8));
  for (int i = 0; i < 8; ++i) WriteDataImmediate(data[i]);
}

}  // namespace pfm2sid
