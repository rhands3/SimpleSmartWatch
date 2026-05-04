#ifndef RTC_TASK_H
#define RTC_TASK_H

extern volatile uint16_t rtc_year;
extern volatile uint8_t rtc_month;
extern volatile uint8_t rtc_day;
extern volatile uint8_t rtc_hour;
extern volatile uint8_t rtc_min;
extern volatile uint8_t rtc_sec;
extern volatile uint8_t rtc_weekday;

void StartRTC(void *argument);

#endif

