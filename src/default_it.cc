// Copyright 2023 Patrick Dowling
//
// Author: Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
// Default interrupt handlers.

#include <stdint.h>

#define EMPTY_HANDLER(x) \
  void x() {}

#define TRAP_HANDLER(x) \
  void x()              \
  {                     \
    for (;;) {}         \
  }

extern "C" {

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  while (1)
    ;
}
#endif

EMPTY_HANDLER(NMI_Handler)

TRAP_HANDLER(HardFault_Handler)
TRAP_HANDLER(MemManage_Handler)
TRAP_HANDLER(BusFault_Handler)
TRAP_HANDLER(UsageFault_Handler)

EMPTY_HANDLER(SVC_Handler)
EMPTY_HANDLER(DebugMon_Handler)
EMPTY_HANDLER(PendSV_Handler)

TRAP_HANDLER(__cxa_pure_virtual)

}  // extern "C"

void *operator new(unsigned int) noexcept
{
  return nullptr;
}

void operator delete(void *) noexcept
{
  while (1) {}
}
void operator delete(void *, unsigned int) noexcept
{
  while (1) {}
}

void *operator new[](unsigned int) noexcept
{
  return nullptr;
}
void operator delete[](void *) noexcept
{
  while (1) {}
}
void operator delete[](void *, unsigned int) noexcept
{
  while (1) {}
}
