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
#pragma once

#include "always.h"
#include "coord.h"
#include "mempoolobj.h"
#include "unicodestring.h"
#include "wininstancedata.h"

class GameFont;

//TODO confirm
enum WindowMsgHandledType
{
    MSG_IGNORED,
    MSG_HANDLED,
};

enum __unnamed_95_
{
    GWS_PUSH_BUTTON = 1 << 0,
    GWS_RADIO_BUTTON = 1 << 1,
    GWS_CHECK_BOX = 1 << 2,
    GWS_VERT_SLIDER = 1 << 3,
    GWS_HORZ_SLIDER = 1 << 4,
    GWS_SCROLL_LIST_BOX = 1 << 5,
    GWS_ENTRY_FIELD = 1 << 6,
    GWS_STATIC_TEXT = 1 << 7,
    GWS_PROGRESS_BAR = 1 << 8,
    GWS_USER = 1 << 9,
    GWS_MOUSE_TRACK = 1 << 10,
    GWS_ANIMATED = 1 << 11,
    GWS_TAB_STOP = 1 << 12,
    GWS_TAB_CONTROL = 1 << 13,
    GWS_TAB_PLANE = 1 << 14,
    GWS_COMBO_BOX = 1 << 15,
};

typedef WindowMsgHandledType(__cdecl *window_callback_func)(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);
typedef void(__cdecl *window_draw_func)(GameWindow *window, WinInstanceData *instance);
typedef void(__cdecl *window_tooltip_func)(GameWindow *window, WinInstanceData *instance, unsigned int mouse);

struct GameWindowEditData
{
    Utf8String system_callback_string;
    Utf8String input_callback_string;
    Utf8String tooltip_callback_string;
    Utf8String draw_callback_string;
};

class GameWindow : public MemoryPoolObject
{
    GameWindow();
    virtual ~GameWindow();


    virtual void Win_Draw_Border() = 0;
    virtual int Win_Set_Text(Utf16String new_text);
    virtual void Win_Set_Font(GameFont *font);

    void Normalize_Window_Region();

protected:
    int m_status;
    ICoord2D m_size;
    IRegion2D m_region;
    int m_cursorX;
    int m_cursorY;
    void *m_userData;
    WinInstanceData m_instData;
    void *m_inputData;
    window_callback_func m_input;
    window_callback_func m_system;
    window_draw_func m_draw;
    window_tooltip_func m_tooltip;
    GameWindow *m_next;
    GameWindow *m_prev;
    GameWindow *m_parent;
    GameWindow *m_child;
    GameWindow *m_nextLayout;
    GameWindow *m_prevLayout;
    GameWindow *m_layout;
    GameWindowEditData *m_editData;
};
