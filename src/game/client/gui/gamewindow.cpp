/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "gamewindow.h"

#include "displaystring.h"

GameWindow::GameWindow() :
    m_status(0),
    m_size(),
    m_region(),
    m_cursorX(0),
    m_cursorY(0),
    m_userData(nullptr),
    m_inputData(nullptr),
    m_input(nullptr),
    m_system(nullptr),
    m_draw(nullptr),
    m_tooltip(nullptr),
    m_next(nullptr),
    m_prev(nullptr),
    m_parent(nullptr),
    m_child(nullptr),
    m_nextLayout(nullptr),
    m_prevLayout(nullptr),
    m_layout(nullptr),
    m_editData(nullptr)
{
    // Win_Set_Draw_Func;
    // Win_Set_Input_Func;
    // Win_Set_System_Func;
    // Win_Set_Tooltip_Func;
}

GameWindow::~GameWindow()
{
    if (m_inputData) {
        delete m_inputData;
    }
    m_inputData = nullptr;

    if (m_editData) {
        delete m_editData;
    }

    m_editData = nullptr;
}

int GameWindow::Win_Set_Text(Utf16String new_text)
{
    m_instData.Set_Text(new_text);

    return 0;
}

void GameWindow::Win_Set_Font(GameFont *font)
{
    m_instData.m_font = font;

    if (m_instData.Get_Style() & GWS_SCROLL_LIST_BOX) {
        //GadgetListBoxSetFont(v2, font);
        return;
    } 

    if (m_instData.Get_Style() & GWS_COMBO_BOX) {
        //GadgetComboBoxSetFont(v2, font);
        return;
    } 

    if (m_instData.Get_Style() & GWS_ENTRY_FIELD) {
        //GadgetTextEntrySetFont(v2, font);
        return;

    }
    if (m_instData.Get_Style() & GWS_STATIC_TEXT) {
        //GadgetStaticTextSetFont(v2, font);
        return;
    }

    DisplayString *t = m_instData.Get_Text_DisplayString();
    if (t) {
        t->Set_Font(font);
    }
    DisplayString *tt = m_instData.Get_Tooltip_DisplayString();
    if (tt) {
        tt->Set_Font(font);
    }
}

void GameWindow::Normalize_Window_Region()
{
    if (m_region.lo.x > m_region.hi.x) {
        int tmp = m_region.lo.x;
        m_region.lo.x = m_region.hi.x;
        m_region.hi.x = tmp;
    }

    if (m_region.lo.y > m_region.hi.y) {
        int tmp = m_region.lo.y;
        m_region.lo.y = m_region.hi.y;
        m_region.hi.y = tmp;
    }
}
