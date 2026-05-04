#ifndef STATE_MENU_H
#define STATE_MENU_H

#include "menu.h"
#include "calendar.h"
#include "oled.h"
#include "font.h"
#include "event_machine.h"


void InitMenus();
uint8_t MenuNav_IsRoot(void);
void DrawAllCards();
void Menu_EnterChild() ;
void Menu_GoBack() ;
void MenuNav_Next() ;
void MenuNav_Prev();
MenuNode* MenuNav_GetByIndex(uint8_t index);

#endif