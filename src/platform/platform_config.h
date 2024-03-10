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
#ifndef PFM2SID_PLATFORM_CONFIG_H_
#define PFM2SID_PLATFORM_CONFIG_H_

#define PFM2SID_OS_STACK_SIZE 256

enum PFM2SID_TIMER_ID { TIMER_UI_TICK, TIMER_COUNT };
enum PFM2SID_TASK_ID { TASK_MAIN, TASK_RENDER, TASK_USBH, TASK_COUNT };

#define TASK_MAIN_TICK_NOTIFY_IDX 0
#define TASK_RENDER_IRQ_NOTIFY_IDX 0

// Assuming NVIC_PriorityGroup_4
#define PFM2SID_CORE_TIMER_IRQ_PRIORITY 0
#define PFM2SID_USB_HS_IRQ_PRIORITY 2
#define PFM2SID_RENDER_IRQ_PRIORITY configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

#define PFM2SID_RENDER_IRQn EXTI0_IRQn
#define PFM2SID_RENDER_IRQ_HANDLER EXTI0_IRQHandler

#define PFM2SID_CORE_TIMER_IRQn TIM1_UP_TIM10_IRQn
#define PFM2SID_CORE_TIMER_IRQ_HANDLER TIM1_UP_TIM10_IRQHandler

// #define PF2SID_USB_FS_ENABLE

#define PFM2SID_USBH_QUEUE_LENGTH 8

#ifdef __cplusplus
extern "C" {
#endif

void vAssertCalled(const char* const, unsigned long);

#ifdef __cplusplus
}
#endif

#endif  // PFM2SID_PLATFORM_CONFIG_H_
