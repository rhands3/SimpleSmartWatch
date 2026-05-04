/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "keycontrol.h"
#include "knob.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for KeyForwardTask */
osThreadId_t KeyForwardTaskHandle;
const osThreadAttr_t KeyForwardTask_attributes = {
  .name = "KeyForwardTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for DisplayTask1 */
osThreadId_t DisplayTask1Handle;
const osThreadAttr_t DisplayTask1_attributes = {
  .name = "DisplayTask1",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RtcTask */
osThreadId_t RtcTaskHandle;
const osThreadAttr_t RtcTask_attributes = {
  .name = "RtcTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for EncoderTask */
osThreadId_t EncoderTaskHandle;
const osThreadAttr_t EncoderTask_attributes = {
  .name = "EncoderTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for KeyTransmitQueue */
osMessageQueueId_t KeyTransmitQueueHandle;
const osMessageQueueAttr_t KeyTransmitQueue_attributes = {
  .name = "KeyTransmitQueue"
};
/* Definitions for EncoderHandle */
osMessageQueueId_t EncoderHandleHandle;
const osMessageQueueAttr_t EncoderHandle_attributes = {
  .name = "EncoderHandle"
};
/* Definitions for KeySem1 */
osSemaphoreId_t KeySem1Handle;
const osSemaphoreAttr_t KeySem1_attributes = {
  .name = "KeySem1"
};
/* Definitions for oled_dma_sem */
osSemaphoreId_t oled_dma_semHandle;
const osSemaphoreAttr_t oled_dma_sem_attributes = {
  .name = "oled_dma_sem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartForward(void *argument);
extern void StartDisplay1(void *argument);
extern void StartRTC(void *argument);
extern void StartEncoder(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of KeySem1 */
  KeySem1Handle = osSemaphoreNew(1, 0, &KeySem1_attributes);

  /* creation of oled_dma_sem */
  oled_dma_semHandle = osSemaphoreNew(1, 1, &oled_dma_sem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of KeyTransmitQueue */
  KeyTransmitQueueHandle = osMessageQueueNew (16, sizeof(key_type), &KeyTransmitQueue_attributes);

  /* creation of EncoderHandle */
  EncoderHandleHandle = osMessageQueueNew (16, sizeof(Knob_message), &EncoderHandle_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of KeyForwardTask */
  KeyForwardTaskHandle = osThreadNew(StartForward, NULL, &KeyForwardTask_attributes);

  /* creation of DisplayTask1 */
  DisplayTask1Handle = osThreadNew(StartDisplay1, NULL, &DisplayTask1_attributes);

  /* creation of RtcTask */
  RtcTaskHandle = osThreadNew(StartRTC, NULL, &RtcTask_attributes);

  /* creation of EncoderTask */
  EncoderTaskHandle = osThreadNew(StartEncoder, NULL, &EncoderTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartForward */
/**
  * @brief  Function implementing the KeyForwardTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartForward */
__weak void StartForward(void *argument)
{
  /* USER CODE BEGIN StartForward */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END StartForward */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

