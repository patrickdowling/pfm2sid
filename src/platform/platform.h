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
#ifndef PFM2SID_PLATFORM_H_
#define PFM2SID_PLATFORM_H_

#include "platform_config.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define ENABLE_WCONVERSION() _Pragma("GCC diagnostic warning \"-Wconversion\"")

#define PRINT_F32(x) static_cast<double>(x)

#ifdef PFM2SID_USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

extern StaticTimer_t xTimerBuffers[PFM2SID_OS_NUM_TIMERS];
extern TimerHandle_t xTimerHandles[PFM2SID_OS_NUM_TIMERS];

extern StaticTask_t xTaskBuffer;
extern StackType_t xTaskStack[PFM2SID_OS_STACK_SIZE];
extern TaskHandle_t xMainTaskHandle;
#endif

#endif
