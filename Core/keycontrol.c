#include "cmsis_os2.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_uart.h"
#include "keycontrol.h"

/* 轮询队列按键逻辑
void StartForward(void *argument)
{
  static uint8_t last_current = 1;
  uint8_t mess[] = "Key1 press";
  key_type state = keyforward;
  for(;;){
    // osSemaphoreAcquire(KeySem1Handle, osWaitForever);
    uint8_t current = HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin);

    if(last_current == 1 && current == 0){
      osDelay(10);
      if(HAL_GPIO_ReadPin(Key1_GPIO_Port,  Key1_Pin) == GPIO_PIN_RESET){
      HAL_UART_Transmit(&huart2, mess, sizeof(mess) - 1, HAL_MAX_DELAY);
      osMessageQueuePut(KeyTransmitQueueHandle, &state, 0, 200);
      }
    }
    last_current = current; 
    osDelay(20);
  }
}

void StartBackward(void *argument)
{
  uint8_t mess2[] = "KEY2 press";
  static uint8_t lastkey2 = 1;
  key_type state = keybackward;
  for(;;)
  {
    // osSemaphoreAcquire(KeySem1Handle, osWaitForever);
    uint8_t current2 = HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin);
    if(lastkey2 == 1 && current2 == 0){
      osDelay(10);
      if(HAL_GPIO_ReadPin(Key2_GPIO_Port,  Key2_Pin) == GPIO_PIN_RESET){
      HAL_UART_Transmit(&huart2, mess2, sizeof(mess2) - 1, HAL_MAX_DELAY);
      osMessageQueuePut(KeyTransmitQueueHandle, &state, 0, 200);
      }
    }
    lastkey2 = current2;
    osDelay(20);
  }
}
*/