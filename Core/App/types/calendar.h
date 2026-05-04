#ifndef CALENDAR_H
#define CALENDAR_H

typedef enum{   //设立修改日历
    Item_year = 0,
    Item_mon,
    Item_day,
    Item_hour,
    Item_min,
    Item_MAX,
}Item_cal;

void EnterSettingCal();
void AdjustCal(int8_t delta);
void SaveAndExit();
void ShowTime();
void ShowTimeSet();


extern Item_cal current_item;


#endif