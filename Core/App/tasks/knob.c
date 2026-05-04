#include "knob.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "keycontrol.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <sys/_intsup.h>


#define RotateCount 2   //编码器旋转
#define long_press  800
#define pressdown 0
#define pressrelease 1

extern volatile uint32_t key3_press_time;
extern volatile uint8_t  key3_pressing;


//思想：只统计差值，正差值即顺时针，负数反之;
void StartEncoder(void *argument)
{ 
    Knob_message mess;
    int32_t last_count = 0;
    int32_t diff_count = 0;
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);

  for(;;){
    //- -------------长按与短按 - HAL_GPIO_EXTI_Callback
    if(key3_pressing){
        uint8_t key3 = HAL_GPIO_ReadPin(Key3_GPIO_Port,  Key3_Pin);
        if(key3 == pressdown && (HAL_GetTick() - key3_press_time >= long_press)){
            mess.type = Knob_Long; 
            osMessageQueuePut(EncoderHandleHandle, &mess, 0, 0);
            key3_pressing = 0;
            while(HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == 0) osDelay(10);
        }
        else if(key3 == pressrelease){
            mess.type = Knob_Short; 
            osMessageQueuePut(EncoderHandleHandle, &mess, 0, 0);
            key3_pressing = 0;
        }
    }


    // ------ -----------      旋转代码           ---------- -----
    int32_t now_count = __HAL_TIM_GET_COUNTER(&htim1);
    diff_count = now_count - last_count;

    if(diff_count >= RotateCount){
        mess.type = Knob_R;
        osMessageQueuePut( EncoderHandleHandle, &mess, 0, 0);
        last_count = now_count;
    }
    if(diff_count <= -RotateCount){
        mess.type = Knob_L;
        osMessageQueuePut( EncoderHandleHandle, &mess, 0, 0);
        last_count = now_count;
    }
    osDelay(10);

  }
}
