#include "cmsis_os2.h"
#include "keycontrol.h"
#include "knob.h"
#include "main.h"
#include "oled.h"
#include "font.h"
#include "rtctask.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include "time.h"
#include "n_rtc.h"
#include "menu.h"

#define gross_menu Menu_enum_MAX 

extern osMutexId_t rtc_mutex;   //rtctasks 互锁


typedef enum{
    State_Normal,
    State_Setting,
    State_Adjusting,
}SystemMode;

typedef enum{   //设立修改日历
    Item_year = 0,
    Item_mon,
    Item_day,
    Item_hour,
    Item_min,
    Item_MAX,
}Item_cal;


//修改日历
uint16_t set_year;
uint8_t set_mon,set_day,set_hour,set_min,set_week;
Item_cal current_item = Item_year;
SystemMode mode = State_Normal;
//应该叫做目前的模式 - MOde ， 要与Current main menu做切割
uint8_t current_meau = 0; 
static key_type state;
static Knob_message state_mess;

//test
// static uint32_t key_count = 0;
// extern UART_HandleTypeDef huart2;
// static uint32_t frame_count = 0;

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

//日历
char date[20]; 
char time_str[20]; 
char *weekdays[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};



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
// frame_count++;
    char buffer[30];
    uint32_t tick = HAL_GetTick();
    uint8_t blink = (tick % 1000) < 500;

    uint8_t normal_color = OLED_COLOR_NORMAL;
    uint8_t selected_color;

    if (mode == State_Adjusting && blink) {
        selected_color = OLED_COLOR_REVERSED;   // 调节 + 闪烁 = 反白
    } else if (mode == State_Adjusting) {
        selected_color = OLED_COLOR_REVERSED;   // 调节模式持续反白
    } else if (blink) {
        selected_color = OLED_COLOR_NORMAL;   // 设置模式闪烁（用下划线）
    } else {
        selected_color = OLED_COLOR_NORMAL;
    }


    //年
    if (blink == 1 && current_item == Item_year && mode == State_Setting) {
        OLED_DrawLine(12, 12, 36, 12, OLED_COLOR_NORMAL);
    } else {
        sprintf_m(set_year, buffer);
        OLED_PrintASCIIString(12, 0, buffer, &afont12x6, (current_item == Item_year) ? selected_color : normal_color);
    }
        OLED_PrintASCIIString(36, 0, "-", &afont12x6, OLED_COLOR_NORMAL);   //保持=

    //月
    if (blink == 1 && current_item == Item_mon && mode == State_Setting) {
        OLED_DrawLine(42, 12, 54, 12, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_mon,2);
        OLED_PrintASCIIString(42, 0, buffer, &afont12x6, (current_item == Item_mon) ? selected_color : normal_color);
    }
        OLED_PrintASCIIString(54, 0, "-", &afont12x6, OLED_COLOR_NORMAL);   //保持=

    //日
    if (blink == 1 && current_item == Item_day && mode == State_Setting) {
        OLED_DrawLine(60, 12, 72, 12, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_day,2);
        OLED_PrintASCIIString(60, 0, buffer, &afont12x6, (current_item == Item_day) ? selected_color : normal_color);
    }

    //周
    OLED_PrintASCIIString(80, 0, weekdays[set_week], &afont12x6, OLED_COLOR_NORMAL);

    //时
    if (blink == 1 && current_item == Item_hour && mode == State_Setting) {
        OLED_DrawLine(16, 32, 40, 32, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_hour,2);
        OLED_PrintASCIIString(16, 20, buffer, &afont24x12, (current_item == Item_hour) ? selected_color : normal_color);
    }
    OLED_PrintASCIIString(40, 20, ":", &afont24x12, OLED_COLOR_NORMAL); //保持:

    //分
    if (blink == 1 && current_item == Item_min && mode == State_Setting) {
        OLED_DrawLine(52, 32, 76, 32, OLED_COLOR_NORMAL);
    } else {
        sprintf_0(buffer, set_min,2);
        OLED_PrintASCIIString(52, 20, buffer, &afont24x12, (current_item == Item_min) ? selected_color : normal_color);
    }

    //模式
        if (mode == State_Setting) {
        OLED_PrintASCIIString(82, 30, "SETMODE", &afont12x6, OLED_COLOR_NORMAL);
    } else {
        OLED_PrintASCIIString(82, 30, "ADJMODE", &afont12x6, OLED_COLOR_REVERSED);
    }
}


// -------------------------------- 基于菜单 Begin -------------------------------------------------




// 在主菜单中循环节点
typedef enum{
    Menu_Time = 0,
    Menu_CountFoot,
    Menu_Test,
    Menu_Test2,
    Menu_enum_MAX,
}Menu_Main_Item;



const int card_x[3] = {0, 44, 88};

//主菜单
static MenuNode menu_node_type_main[Menu_enum_MAX];
static Menu_Main_Item current_main_menu = Menu_Time;

//只是为了添加结构体
static MenuNode Menu_Adjust;
static MenuNode Menu_Settings;

void SwitchToMenu(Menu_Main_Item type) {
        current_main_menu = type;
        Menu_Switch(&menu_node_type_main[type]);
    }

const int card_w = 44;


// on draw
static void on_draw_time() {

    ShowTime();
}

static void on_draw_settings() {

    ShowTimeSet();
}

static void on_draw_adjust() {

    ShowTimeSet();
}

static void on_draw_CountFoot(int x_off) {
    OLED_DrawImage(x_off + 10, 24, &foot_image, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(x_off + 4, 8, "STEP", &afont12x6, OLED_COLOR_NORMAL);
}

static void on_draw_Test(int x_off) {
    OLED_DrawImage(x_off + 10, 24, &rrrrun, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(x_off + 4, 8, "TEST", &afont12x6, OLED_COLOR_NORMAL);
}

static void on_draw_Test2(int x_off) {
    OLED_DrawImage(x_off + 10, 24, &rrrrun, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(x_off + 4, 8, "TST2", &afont12x6, OLED_COLOR_NORMAL);
}


// -----------------------------------------------------------------------------on key ----------------------------------------------------------------------------------
static void on_key_time(int state) {
    
    if(mode == State_Normal)
    {
        // 主菜单：切页
        if (state == keyforward){
          current_meau = (current_meau + 1) % gross_menu;

          
        }
        else{
          current_meau = (current_meau - 1 + gross_menu) % gross_menu;
        //   Menu_Switch(menu_mgr.current->prev);
        }
        current_main_menu = current_meau;
      }
}

static void on_key_settings(int state){ //注:这里的state不是全局变量的state

    if (mode == State_Setting)
      {
        // 设置模式：切设置项
        if (state == keyforward){
            current_item = (current_item + 1) % Item_MAX;
        }
        else{
            current_item = (current_item - 1 + Item_MAX) % Item_MAX;
        }
      }

}

static void on_key_adjust(int state){
    
    if (mode == State_Adjusting)
      {
        // 年月日时分秒 item_max
        // 调节模式：KEY1/KEY2 调下一个选项
        if (state == keyforward){
            AdjustCal(1);
        }
        else{
            AdjustCal(-1);
        }
      }
}

// 菜单系统
void InitMenus() {
    Menu_Init();
     // 时间菜单
    menu_node_type_main[Menu_Time].title = "Time";
    menu_node_type_main[Menu_Time].on_enter = NULL;
    menu_node_type_main[Menu_Time].on_exit = NULL;
    menu_node_type_main[Menu_Time].on_draw = on_draw_time;
    menu_node_type_main[Menu_Time].on_key = on_key_time;
    

    menu_node_type_main[Menu_CountFoot].title = "CountFoot";
    menu_node_type_main[Menu_CountFoot].on_enter = NULL;   // 进入时加载当前时间
    menu_node_type_main[Menu_CountFoot].on_exit = NULL;        // 退出时保存
    menu_node_type_main[Menu_CountFoot].on_draw = on_draw_CountFoot;
    menu_node_type_main[Menu_CountFoot].on_key = on_key_time;

    menu_node_type_main[Menu_Test].title = "CountFoot";
    menu_node_type_main[Menu_Test].on_enter = NULL;   // 进入时加载当前时间
    menu_node_type_main[Menu_Test].on_exit = NULL;        // 退出时保存
    menu_node_type_main[Menu_Test].on_draw = on_draw_Test;
    menu_node_type_main[Menu_Test].on_key = on_key_time;

    menu_node_type_main[Menu_Test2].title = "CountFoot";
    menu_node_type_main[Menu_Test2].on_enter = NULL;   // 进入时加载当前时间
    menu_node_type_main[Menu_Test2].on_exit = NULL;        // 退出时保存
    menu_node_type_main[Menu_Test2].on_draw = on_draw_Test2;
    menu_node_type_main[Menu_Test2].on_key = on_key_time;

        // -------------------------------子菜单
    Menu_Settings = (MenuNode){
        .title = "Settings",
        .on_enter = EnterSettingCal,
        .on_exit = SaveAndExit,
        .on_draw = on_draw_settings,
        .on_key = on_key_settings,
        .prev = &Menu_Adjust,
        .next = &Menu_Adjust
    };
    
    Menu_Adjust = (MenuNode){
        .title = "Adjust",
        .on_enter = NULL,
        .on_exit = NULL,
        .on_draw = on_draw_adjust,
        .on_key = on_key_adjust,
        .prev = &Menu_Settings,
        .next = &Menu_Settings
    };
    
    // 添加到链表
    for (int i = 0; i < Menu_enum_MAX; i++) {
        Menu_AddNewNode(&menu_node_type_main[i]);
    }

    // 设置当前菜单为时间
    menu_mgr.current = &menu_node_type_main[Menu_Time];
}


// -------------------------------- 基于菜单 ENd -------------------------------------------------


void StartDisplay1(void *argument)
{

  OLED_Init();
  osDelay(20);
  InitMenus();
  uint32_t last_refresh = 0; 
  uint8_t need_refresh = 0;



  for(;;)   //一切接收多条消息导致过慢，使用while代替if
  {

    Menu_UpdateAnimation();
    //Key1 key2
    while(osMessageQueueGet(KeyTransmitQueueHandle, &state, NULL, 0)  == osOK){
// key_count++;
        need_refresh = 1;
        if (menu_mgr.current->on_key) {
        menu_mgr.current->on_key(state); 
    }
    osDelay(15);
}

    //编码器
    while((osMessageQueueGet(EncoderHandleHandle, &state_mess, NULL, 0)  == osOK)){
// key_count++;
        if (state_mess.type == Knob_Long){
            if (mode == State_Setting){
                SaveAndExit();
                mode = State_Normal;
                menu_mgr.current = &menu_node_type_main[Menu_Time];
                current_main_menu = Menu_Time;
            }
            else if (mode == State_Normal){
                EnterSettingCal();
                mode = State_Setting;
                menu_mgr.current = &Menu_Settings;
            }
        }

        // 短按：设置退出/进入调节
        if (state_mess.type == Knob_Short){
            if (mode == State_Setting){
                mode = State_Adjusting;
                menu_mgr.current = &Menu_Adjust;
            }
            else if (mode == State_Adjusting){
                mode = State_Setting;
                menu_mgr.current = &Menu_Settings;
                current_main_menu = Menu_Time;
            }
        }

            //  编码器
        if (mode == State_Adjusting){
                if(state_mess.type == Knob_R){
                    AdjustCal(1);
                }
                else if(state_mess.type == Knob_L){
                    AdjustCal(-1);
                }
            }
        
        }

    //OLED显示
    uint32_t refresh_interval;
    if (menu_mgr.anim_active) {
        refresh_interval = 0; 
    }else if (mode == State_Setting || mode == State_Adjusting) {
    refresh_interval = 50;
    } else if (current_main_menu == Menu_CountFoot) {
        refresh_interval = 150; 
    } else {
        refresh_interval = 300;
    }

    if((mode == State_Normal && need_refresh == 1) || (HAL_GetTick() - last_refresh) >= refresh_interval){     
        
        //test
//         uint8_t diag[30];
// int len = snprintf((char*)diag, sizeof(diag), "K:%lu F:%lu", key_count, frame_count);

// if (len > 0 && len < sizeof(diag)) {
//     HAL_UART_Transmit(&huart2, diag, len, HAL_MAX_DELAY);
// }

        OLED_NewFrame();

        if (menu_mgr.anim_active) {
              // 画两页
        }else if(mode == State_Setting || mode == State_Adjusting){
            ShowTimeSet();
        }else if(current_main_menu == Menu_Time){
            on_draw_time();
        }else {
            // 卡片模式：遍历画所有卡片
            for (int i = 1; i < Menu_enum_MAX; i++) {
                MenuNode *node = &menu_node_type_main[i];
                if (node->on_draw) {
                    node->on_draw(card_x[i - 1]);
                }
            }
        // 折线框只画在选中的卡片上
             int slot = current_main_menu  - 1;
             if (slot >= 0 && slot < 3) {
                 DrawCornerDecorator(slot);  // 传索引
             }
        }
        OLED_ShowFrame();
        last_refresh = HAL_GetTick();
        need_refresh = 0; 
        }
    osDelay(5);
   } 
}