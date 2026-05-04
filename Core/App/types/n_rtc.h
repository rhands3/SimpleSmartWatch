#ifndef N_RTC_H
#define N_RTC_H

#include "stm32f1xx_hal.h"
#include "rtc.h"
#include "time.h"

struct tm *Watch_RTC_GetTime();
HAL_StatusTypeDef Watch_RTC_SetTime(struct tm *time);
void Watch_RTC_Init();

#endif