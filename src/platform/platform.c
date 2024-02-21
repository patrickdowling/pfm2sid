#include "platform.h"

#ifdef PFM2SID_USE_FREERTOS

StaticTimer_t xTimerBuffers[PFM2SID_OS_NUM_TIMERS] __attribute__((section(".ccmz")));
TimerHandle_t xTimerHandles[PFM2SID_OS_NUM_TIMERS] __attribute__((section(".ccmz")));

StaticTask_t xTaskBuffer __attribute__((section(".ccmz")));
StackType_t xTaskStack[PFM2SID_OS_STACK_SIZE] __attribute__((section(".ccmz")));
TaskHandle_t xMainTaskHandle __attribute__((section(".ccmz")));
StaticTask_t xIdleTaskTCBBuffer;

static StackType_t xIdleTaskStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *puxIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = xIdleTaskStack;
  *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

StaticTask_t xTimerTaskTCBBuffer;
StackType_t xTimerTaskStackBuffer[configMINIMAL_STACK_SIZE];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *puxTimerTaskStackSize)
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = xTimerTaskStackBuffer;
  *puxTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vAssertCalled(const char *const file, unsigned long line)
{
  (void)file;
  (void)line;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
}

#endif
