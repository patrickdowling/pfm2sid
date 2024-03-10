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
#include <cstdio>

#include "drivers/lcd.h"
#include "platform.h"

extern "C" void assert_failed(const char *const file, unsigned long line)
    __attribute__((weak, alias("vAssertCalled")));

static char assert_buf[21];

extern "C" void vAssertCalled(const char *const file, unsigned long line)
{
  taskDISABLE_INTERRUPTS();

  auto len = strlen(file);
  snprintf(assert_buf, sizeof(assert_buf), "%-20s", len > 20 ? file + (len - 20) : file);
  pfm2sid::lcd.MoveCursor(0, 0);
  pfm2sid::lcd.Print(assert_buf);

  snprintf(assert_buf, sizeof(assert_buf), "line %lu", line);
  pfm2sid::lcd.MoveCursor(1, 0);
  pfm2sid::lcd.Print(assert_buf);

  for (;;) {}
}
