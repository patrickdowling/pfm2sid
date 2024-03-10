#include "platform.h"

#include "platform_config.h"

StaticTimer_t xTimerBuffers[TIMER_COUNT] __attribute__((section(".ccmz")));
TimerHandle_t xTimerHandles[TIMER_COUNT] __attribute__((section(".ccmz")));

StaticTask_t xTaskBuffer[TASK_COUNT] __attribute__((section(".ccmz")));
StackType_t xTaskStack[TASK_COUNT][PFM2SID_OS_STACK_SIZE] __attribute__((section(".ccmz")));
TaskHandle_t xTaskHandles[TASK_COUNT] __attribute__((section(".ccmz")));
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

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
}

static const struct {
  const char task_name[configMAX_TASK_NAME_LEN];
  uint32_t task_stack_size;
  UBaseType_t task_priority;
} task_defs[TASK_COUNT] = {
    {"MAIN", PFM2SID_OS_STACK_SIZE, tskIDLE_PRIORITY},
    {"SAMPLES", PFM2SID_OS_STACK_SIZE, configTIMER_TASK_PRIORITY - 1},
    {"USBH", PFM2SID_OS_STACK_SIZE, tskIDLE_PRIORITY},
};

TaskHandle_t TaskCreate(enum PFM2SID_TASK_ID task_id, TaskFunction_t pxTaskCode, void *pvParameters)
{
  configASSERT(task_id < TASK_COUNT);

  xTaskHandles[task_id] = xTaskCreateStatic(
      pxTaskCode, task_defs[task_id].task_name, task_defs[task_id].task_stack_size, pvParameters,
      task_defs[task_id].task_priority, xTaskStack[task_id], &xTaskBuffer[task_id]);

  configASSERT(xTaskHandles[task_id]);
  return xTaskHandles[task_id];
}

static const struct {
  const char *timer_name;
} timer_defs[TIMER_COUNT] = {
    {"UI"},
};

TimerHandle_t TimerCreate(enum PFM2SID_TIMER_ID timer_id, TickType_t xTimerPeriod,
                          TimerCallbackFunction_t pxCallbackFunction)
{
  configASSERT(timer_id < TIMER_COUNT);

  xTimerHandles[timer_id] =
      xTimerCreateStatic(timer_defs[timer_id].timer_name, xTimerPeriod, pdTRUE, (void *)0,
                         pxCallbackFunction, &xTimerBuffers[timer_id]);
  configASSERT(xTimerHandles[timer_id]);
  return xTimerHandles[timer_id];
}
