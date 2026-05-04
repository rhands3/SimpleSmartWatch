#include "state_menu.h"

#define gross_menu 4  //主菜单数

static MenuNode node_time, node_setting, node_adjust;   //子菜单
static MenuNode node_countfoot;
static MenuNode node_test;
static MenuNode node_test2;



const int card_x[3] = {0, 44, 88};


// --------------------------------------------------------------on draw-------------------------------------------------------------------------------------------------
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
// --------------------------------------------------------------end draw-------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------on key ----------------------------------------------------------------------------------

// --------------------------------------------------------------导航-----------------------------------------------------------

// 时间
// 进入子菜单
void Menu_EnterChild() {
    if (menu_mgr.current && menu_mgr.current->child) {
        menu_mgr.current = menu_mgr.current->child;
        if (menu_mgr.current->on_enter) menu_mgr.current->on_enter();
    }
}

// 返回父菜单
void Menu_GoBack() {
    if (menu_mgr.current && menu_mgr.current->parent) {
        if (menu_mgr.current->on_exit) menu_mgr.current->on_exit();
        menu_mgr.current = menu_mgr.current->parent;
    }
}

// 主菜单
// 同级切换
void MenuNav_Next() {
    if (menu_mgr.current && menu_mgr.current->next) {
        menu_mgr.current = menu_mgr.current->next;
    }
}

void MenuNav_Prev() {
    if (menu_mgr.current && menu_mgr.current->prev) {
        menu_mgr.current = menu_mgr.current->prev;
    }
}

// 时间主页面 — 按键用来切换卡片
static void on_key_time(int key) {
    if (key == keyforward) MenuNav_Next();
    else MenuNav_Prev();
}

// 设置页 — 按键用来切换设置项
static void on_key_settings(int key) {
    if (key == keyforward) current_item = (current_item + 1) % Item_MAX;
    else current_item = (current_item - 1 + Item_MAX) % Item_MAX;
}

// 调节页 — 按键用来加减值
static void on_key_adjust(int key) {
    if (key == keyforward) AdjustCal(1);
    else AdjustCal(-1);
}

// --------------------------------------------------------------导航 END-----------------------------------------------------------

// -----------------------------------------------------------------------------end key ---------------------------------------------------------------------------------

// 菜单初始化。新菜单需在此处填写。
void InitMenus(){
    Menu_Init();
    // ===== 根 =====
    // Time
    node_time = (MenuNode){
        .title = "Time", .on_draw = on_draw_time,.on_key = on_key_time,
        .parent = NULL, .child = &node_setting,.index = 0,
    };
    
    // CountFoot
    node_countfoot = (MenuNode){
        .title = "CountFoot", .on_draw = on_draw_CountFoot,.on_key = on_key_time,
        .parent = NULL, .child = NULL,.index = 1,
    };
    
    // test2
    node_test = (MenuNode){
        .title = "Test2", .on_draw = on_draw_Test,.on_key = on_key_time,
        .parent = NULL, .child = NULL,.index = 2,
    };
    
    // Test
    node_test2 = (MenuNode){
        .title = "Test", .on_draw = on_draw_Test2,.on_key = on_key_time,
        .parent = NULL, .child = NULL,.index = 3,
    };
    
    // 主菜单串成环
    Menu_AddNewNode(&node_time);
    Menu_AddNewNode(&node_countfoot);
    Menu_AddNewNode(&node_test2);
    Menu_AddNewNode(&node_test);
    
    // ===== Time的儿子 =====
    node_setting = (MenuNode){
        .title = "Settings", .on_draw = on_draw_settings,.on_key = on_key_settings,
        .on_enter = EnterSettingCal, .on_exit = SaveAndExit,
        .parent = &node_time, .child = &node_adjust,.index = 99,
    };
    node_adjust = (MenuNode){
        .title = "Adjust", .on_draw = on_draw_adjust,.on_key = on_key_adjust,
        .parent = &node_setting, .child = NULL,.index = 98,
    };

    // 当前在主菜单
    menu_mgr.current = &node_time;
}

// 判断是否为跟 ， 即主菜单
uint8_t MenuNav_IsRoot(void) {
    return menu_mgr.current && menu_mgr.current->parent == NULL;
}

void DrawAllCards() {
    // 遍历主菜单的所有节点，跳过第一个（Time）
    MenuNode *node = menu_mgr.head;
    if (!node) return;
    
    int index = 0;
    do {
        if (node != &node_time && node->on_draw) {
            node->on_draw(card_x[index]);
            index++;
        }
        node = node->next;
    } while (node != menu_mgr.head);
    
    // 高亮选中的卡片（折线框）
    // 找到当前节点在卡片中的索引
    int slot = 0;
    MenuNode *n = menu_mgr.head->next;  // 跳过 Time
    while (n != menu_mgr.current && n != menu_mgr.head) {
        n = n->next;
        slot++;
    }
    if (slot < 3) {
        DrawCornerDecorator(slot);
    }
}


// 查找函数 - 用于调出索引 - 无法查找子菜单
MenuNode* MenuNav_GetByIndex(uint8_t index) {
    MenuNode *node = menu_mgr.head;
    if (!node) return NULL;
    
    do {
        if (node->index == index) return node;
        node = node->next;
    } while (node != menu_mgr.head);
    
    return NULL;
}

