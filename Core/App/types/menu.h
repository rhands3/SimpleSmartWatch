#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "keycontrol.h"

// 菜单节点结构体
typedef struct MenuNode {
    // 链表指针
    struct MenuNode *prev;
    struct MenuNode *next;

     // 父子关系
    struct MenuNode *parent;    // 父节点，NULL = 主菜单
    struct MenuNode *child;     // 第一个子节点，NULL = 无子菜单
    
    // 菜单信息
    char *title;                    // 菜单标题
    void (*on_enter)(void);         // 进入菜单时调用
    void (*on_exit)(void);          // 退出菜单时调用
    void (*on_draw)(int x_off);     // 绘制函数
    void (*on_key)(int key);        // 按键处理
    uint8_t index;                  //索引
    // 动画相关
    int16_t x_offset;               // 当前 X 偏移（用于滑动动画）
    uint8_t is_animating;           // 是否正在动画
} MenuNode;

// 菜单管理器
typedef struct {
    MenuNode *current;              // 当前显示的菜单
    MenuNode *head;                 // 链表头（用于遍历）
    uint32_t node_count;            // 节点数量
    
    // 动画状态
    uint8_t anim_active;
    int16_t anim_start;
    int16_t anim_target;
    uint32_t anim_last_time;
} MenuManager;

// 全局菜单管理器
extern MenuManager menu_mgr;

// 菜单操作函数
void Menu_Init();
void Menu_AddNewNode(MenuNode *node);
void Menu_Switch(MenuNode *target);
void Menu_UpdateAnimation();
int sprintf_m(int num, char *buf);
void sprintf_0(char *buf, int num, int width);

#endif