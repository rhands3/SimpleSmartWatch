/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "cmsis_os2.h"
#include "stm32f1xx_hal.h"
#include "main.h"
#include "keycontrol.h"
#include <stdint.h>
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
volatile uint32_t key3_press_time = 0;
volatile uint8_t  key3_pressing = 0;
extern UART_HandleTypeDef huart2;
/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : Key1_Pin */
  GPIO_InitStruct.Pin = Key1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Key1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Key2_Pin Key3_Pin */
  GPIO_InitStruct.Pin = Key2_Pin|Key3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t last_tick = 0;
  uint32_t now = HAL_GetTick();
  // uint8_t mess2[30] = "Transmit Complete!";
 

  if((now - last_tick) < 10) return;
  last_tick = now;

  if (GPIO_Pin == Key1_Pin) {
    key_type state = keyforward;
    // HAL_UART_Transmit(&huart2, mess2, sizeof(mess2) - 1, 1);
    osMessageQueuePut(KeyTransmitQueueHandle, &state, 0, 0);
  }
  else if (GPIO_Pin == Key2_Pin) {
    key_type state = keybackward;
    // HAL_UART_Transmit(&huart2, mess2, sizeof(mess2) - 1, 1);
    osMessageQueuePut(KeyTransmitQueueHandle, &state, 0, 0);
  }

  else if(GPIO_Pin == Key3_Pin) {
    key3_press_time = now;
    key3_pressing = 1;
  }
  
}
/* USER CODE END 2 */
