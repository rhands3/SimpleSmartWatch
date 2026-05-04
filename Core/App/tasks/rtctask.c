#include "stm32f1xx_hal.h"
#include "rtc.h"
#include "time.h"
#include "n_rtc.h"

osMutexId_t rtc_mutex;

volatile uint16_t rtc_year;
volatile uint8_t rtc_month;
volatile uint8_t rtc_day;
volatile uint8_t rtc_hour;
volatile uint8_t rtc_min;
volatile uint8_t rtc_sec;
volatile uint8_t rtc_weekday;

void StartRTC(void *argument)
{
  struct tm *date;
  Watch_RTC_Init();
  for(;;)
  {
    osMutexAcquire(rtc_mutex, osWaitForever);
    date = Watch_RTC_GetTime();
    rtc_year = date->tm_year + 1900;
    rtc_month = date->tm_mon + 1;
    rtc_day = date->tm_mday;
    rtc_hour = date->tm_hour;
    rtc_min = date->tm_min;
    rtc_sec = date->tm_sec;
    rtc_weekday = date->tm_wday;
    osMutexRelease(rtc_mutex);
    // sprintf(day , "%d-%d-%d", date -> tm_year + 1900,date->tm_mon + 1 , date -> tm_mday);
    // sprintf(time , "%02d : %02d : %02d" , date -> tm_hour , date -> tm_min , date -> tm_sec);
    // char *week = char[date->tm_mday];
    // uint8_t x_week = (128 - (strlen(week)  * 8 )) / 2;
    osDelay(1000);
  }
}