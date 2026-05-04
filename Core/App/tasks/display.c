#include "display.h"
#include "state_menu.h"



static key_type state;
static Knob_message state_mess;




void StartDisplay1(void *argument)
{

  OLED_Init();
  osDelay(20);
  InitMenus();
  uint32_t last_refresh = 0; 




  for(;;)   //一切接收多条消息导致过慢，使用while代替if
  {

    Menu_UpdateAnimation();

    uint8_t need_refresh = 0;

    //Key1 key2
    while(osMessageQueueGet(KeyTransmitQueueHandle, &state, NULL, 0)  == osOK){

        need_refresh = 1;

        if (menu_mgr.current->on_key) {
        menu_mgr.current->on_key(state); 
        }
        osDelay(15);
    }



        // 编码器
    while(osMessageQueueGet(EncoderHandleHandle, &state_mess, NULL, 0) == osOK)
    {
        if (state_mess.type == Knob_Long) {
            // 长按：进入 / 退出设置
            if (menu_mgr.current == MenuNav_GetByIndex(0)) {
        
                Menu_EnterChild();
            } else {
    
                Menu_GoBack();
            }
        }
        else if (state_mess.type == Knob_Short) {
            // 短按：进入 / 退出调节
            if (menu_mgr.current && menu_mgr.current->index == 99) {
                Menu_EnterChild();
            } else if (menu_mgr.current && menu_mgr.current->index == 98) {
                Menu_GoBack();
            }
        }
        else if (state_mess.type == Knob_R) {

                if (menu_mgr.current->on_key) {
                    menu_mgr.current->on_key(keyforward);
                }
            }
        else if (state_mess.type == Knob_L) {
    
                if (menu_mgr.current->on_key) {
                    menu_mgr.current->on_key(keybackward);
                }
            }

    }

    //OLED显示
    uint32_t refresh_interval;
   
        if (menu_mgr.anim_active) {
            refresh_interval = 0;
        } else if (!MenuNav_IsRoot()) {
            // 子菜单：快速刷新
            refresh_interval = 50;
        } else {
            // 主菜单：正常刷新
            refresh_interval = 300;
        }
    
    if( need_refresh || (HAL_GetTick() - last_refresh) >= refresh_interval){     
        
        OLED_NewFrame();

            if (menu_mgr.anim_active) {
                // 动画中，画两页（暂时留空）
            } else if (menu_mgr.current == MenuNav_GetByIndex(0)) {

                ShowTime();
            } else if (MenuNav_IsRoot()) {

                DrawAllCards();
            } else if (menu_mgr.current && menu_mgr.current->on_draw) {

                menu_mgr.current->on_draw(menu_mgr.current->x_offset);
            }
      
            OLED_ShowFrame();
            last_refresh = HAL_GetTick();
            need_refresh = 0;
        }
     osDelay(5);
    }
}