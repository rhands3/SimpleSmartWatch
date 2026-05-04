#ifndef MACHEVENT_H
#define MACHEVENT_H

typedef enum{
    State_Normal,
    State_Setting,
    State_Adjusting,
}SystemMode;

// 在主菜单中循环节点
typedef enum{
    Menu_Time = 0,
    Menu_CountFoot,
    Menu_Test,
    Menu_Test2,
    Menu_enum_MAX,
}Menu_Main_Item;



#endif