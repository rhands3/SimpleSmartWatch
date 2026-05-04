#include "menu.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

MenuManager menu_mgr;

//垃圾文件清楚+记录时间
void Menu_Init() {  
    memset(&menu_mgr, 0, sizeof(MenuManager));
    menu_mgr. anim_last_time = HAL_GetTick();
}

void Menu_AddNewNode(MenuNode *node){
    if(menu_mgr. head == NULL){
        menu_mgr.head = node;
        node->prev = node;
        node -> next = node;
    }
    else{
        MenuNode *tail = menu_mgr.head->prev;
        tail->next = node;
        node->prev = tail;
        node->next = menu_mgr.head;
        menu_mgr.head->prev = node;
    }
    (menu_mgr. node_count)++;
    //第一个菜单为当前菜单
    if(menu_mgr. current == NULL){
        menu_mgr.current = menu_mgr.head;
    }
}

void Menu_Switch(MenuNode *target){
    if(target == NULL || target == menu_mgr. current) return;

    //方向
    int8_t direction = 0;
    if(menu_mgr. current -> next == target){
        direction = -1;  //右
    }else{
        direction =  1;
    }

    if(menu_mgr. current -> on_exit){
        menu_mgr. current -> on_exit();
    }

    menu_mgr.current->is_animating = 0;

     // 全局菜单管理
    menu_mgr.anim_active = 1;
    menu_mgr.anim_start = 0;
    menu_mgr.anim_target =  (direction == -1) ? -128 : 128;
    menu_mgr.anim_last_time = HAL_GetTick();
    menu_mgr. current = target;

    menu_mgr.current->is_animating = 1;

    if(menu_mgr. current -> on_enter){
        menu_mgr. current -> on_enter();
    }
}

void Menu_UpdateAnimation() {
    if (!menu_mgr.anim_active) return;
    
    uint32_t now = HAL_GetTick();
    if (now - menu_mgr.anim_last_time >= 16) {  // 60 FPS
        menu_mgr.anim_last_time = now;
        
        //滑动速度
        int16_t step = 12;
        //如果是往左滑
        if(menu_mgr.anim_start >  menu_mgr. anim_target){
            menu_mgr. anim_start -= step;
            if(menu_mgr. anim_start <=  menu_mgr. anim_target){
                menu_mgr. anim_start = menu_mgr. anim_target;
                menu_mgr.anim_active = 0;
                menu_mgr.current->is_animating = 0;
            }
        }else if(menu_mgr.anim_start <  menu_mgr. anim_target){
            menu_mgr. anim_start += step;
            if(menu_mgr. anim_start >=  menu_mgr. anim_target){
                menu_mgr. anim_start = menu_mgr. anim_target;
                menu_mgr.anim_active = 0;
                menu_mgr.current->is_animating = 0;
            }
        }
        
        //更新当前菜单的偏移量
        menu_mgr.current->x_offset = menu_mgr.anim_start;
    }
}


// 整数转字符串，返回字符串长度 - 
//buf填写char[]，num填写要转义的变量。
int sprintf_m(int num, char *buf)
{
    int i = 0;
    if (num < 0) { buf[i++] = '-'; num = -num; }
    if (num == 0) { buf[i++] = '0'; buf[i] = '\0'; return i; }
    
    char tmp[12];
    int j = 0;
    while (num > 0) { tmp[j++] = '0' + (num % 10); num /= 10; }
    while (j > 0) buf[i++] = tmp[--j];
    buf[i] = '\0';
    return i;
}

// 填充前导零，比如 3 → "03" = 
//buf填写char[]，num填写要转义的变量。width填写变量最大长度
void sprintf_0(char *buf, int num, int width)
{
    int len = sprintf_m(num, buf);
    if (len < width) {
        memmove(buf + width - len, buf, len + 1);
        memset(buf, '0', width - len);
    }
}