#include "n_rtc.h"
#include "rtctask.h"
#include "calendar.h"
#include "main.h"
#include "oled.h"
#include "font.h"
#include "menu.h"
#include "event_machine.h"
#include "time.h"

//修改日历
uint16_t set_year;
uint8_t set_mon,set_day,set_hour,set_min,set_week;
Item_cal current_item = Item_year;
//日历
char date[20]; 
char time_str[20]; 
const char *weekdays[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

extern osMutexId_t rtc_mutex;   //rtctasks 互锁

void EnterSettingCal(){
    set_year = rtc_year;
    set_mon = rtc_month;
    set_day = rtc_day;
    set_hour = rtc_hour;
    set_min = rtc_min;
    set_week = rtc_weekday;
}

void AdjustCal(int8_t delta){
    switch(current_item){
        case Item_year :
        set_year = set_year + delta;
        break;

        case Item_mon :
        set_mon = (set_mon + delta - 1 + 12) % 12 + 1;
        break;

        case Item_day :
        set_day = (set_day + delta - 1 + 31) % 31 + 1;
        break;

        case Item_hour :
        set_hour = (set_hour + delta + 24) % 24;
        break;

        case Item_min :
        set_min = (set_min + delta + 60) % 60;
        break;
        default: break;
    }
}

void SaveAndExit() {
  struct tm time;
  time.tm_year = set_year - 1900;
  time.tm_mon = set_mon - 1;
  time.tm_mday = set_day;
  time.tm_hour = set_hour;
  time.tm_min = set_min;
  time.tm_sec = 0;      // 秒清零
  Watch_RTC_SetTime(&time);
}





void ShowTime(){
    int16_t x_off = menu_mgr.current->x_offset;
// frame_count++;
    osMutexAcquire(rtc_mutex, osWaitForever);
    sprintf_m(rtc_year, date);
    date[4] = '-';
    sprintf_0(date+5, rtc_month,2);
    date[7] = '-';
    sprintf_0(date+8, rtc_day,2);
    date[10] = '\0';
    sprintf_0(time_str, rtc_hour,2);
    time_str[2] = ':';
    sprintf_0(time_str+3, rtc_min,2);
    time_str[5] = ':';
    sprintf_0(time_str+6, rtc_sec,2);
    time_str[8] = '\0';

    osMutexRelease(rtc_mutex);
    OLED_PrintASCIIString(12 + x_off, 0, date, &afont12x6, OLED_COLOR_NORMAL); 
    OLED_PrintASCIIString(16 + x_off, 20, time_str, &afont24x12, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(80 + x_off, 0, weekdays[rtc_weekday], &afont12x6, OLED_COLOR_NORMAL);
}

void ShowTimeSet(){

    char buffer[16];
    uint32_t tick = HAL_GetTick();
    uint8_t blink = (tick % 1000) < 500;

    uint8_t normal_color = OLED_COLOR_NORMAL;
    uint8_t selected_color;

    uint8_t is_adjusting = (menu_mgr.current && menu_mgr.current->child == NULL);
    uint8_t is_setting = (menu_mgr.current && menu_mgr.current->child != NULL);

    if (is_adjusting && blink) {
        selected_color = OLED_COLOR_REVERSED;   // 调节 + 闪烁 = 反白
    } else if (is_adjusting) {
        selected_color = OLED_COLOR_REVERSED;   // 调节模式持续反白
    } else if (blink) {
        selected_color = OLED_COLOR_NORMAL;   // 设置模式闪烁（用下划线）
    } else {
        selected_color = OLED_COLOR_NORMAL;
    }


    //年
    if (blink == 1 && current_item == Item_year && is_setting) {
        OLED_DrawLine(12, 12, 36, 12, OLED_COLOR_NORMAL);
    } else {
        sprintf_m(set_year, buffer);
        OLED_PrintASCIIString(12, 0, buffer, &afont12x6, (current_item == Item_year) ? selected_color : normal_color);
    }
        OLED_PrintASCIIString(36, 0, "-", &afont12x6, OLED_COLOR_NORMAL);   //保持=

    //月
    if (blink == 1 && current_item == Item_mon && is_setting) {
        OLED_DrawLine(42, 12, 54, 12, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_mon,2);
        OLED_PrintASCIIString(42, 0, buffer, &afont12x6, (current_item == Item_mon) ? selected_color : normal_color);
    }
        OLED_PrintASCIIString(54, 0, "-", &afont12x6, OLED_COLOR_NORMAL);   //保持=

    //日
    if (blink == 1 && current_item == Item_day && is_setting) {
        OLED_DrawLine(60, 12, 72, 12, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_day,2);
        OLED_PrintASCIIString(60, 0, buffer, &afont12x6, (current_item == Item_day) ? selected_color : normal_color);
    }

    //周
    OLED_PrintASCIIString(80, 0, weekdays[set_week], &afont12x6, OLED_COLOR_NORMAL);

    //时
    if (blink == 1 && current_item == Item_hour && is_setting) {
        OLED_DrawLine(16, 32, 40, 32, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_hour,2);
        OLED_PrintASCIIString(16, 20, buffer, &afont24x12, (current_item == Item_hour) ? selected_color : normal_color);
    }
    OLED_PrintASCIIString(40, 20, ":", &afont24x12, OLED_COLOR_NORMAL); //保持:

    //分
    if (blink == 1 && current_item == Item_min && is_setting) {
        OLED_DrawLine(52, 32, 76, 32, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_min,2);
        OLED_PrintASCIIString(52, 20, buffer, &afont24x12, (current_item == Item_min) ? selected_color : normal_color);
    }

    //模式
        if (is_setting) {
        OLED_PrintASCIIString(82, 30, "SETMODE", &afont12x6, OLED_COLOR_NORMAL);
    } else {
        OLED_PrintASCIIString(82, 30, "ADJMODE", &afont12x6, OLED_COLOR_REVERSED);
    }
}