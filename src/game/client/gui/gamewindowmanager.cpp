/**
 * @file
 *
 * @author tomsons26
 *
 * @brief WND UI system manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamewindowmanager.h"

#include "color.h"
#include "display.h"
#include "gamefont.h"

GameWindowManager::GameWindowManager() :
    m_windowList(nullptr),
    m_windowTail(nullptr),
    m_destroyList(nullptr),
    m_currMouseRgn(nullptr),
    m_mouseCaptor(nullptr),
    m_keyboardFocus(nullptr),
    m_modalHead(nullptr),
    m_grabWindow(nullptr),
    m_loneWindow(nullptr),
    m_cursorBitmap(nullptr),
    m_captureFlags(0)
{
}

GameWindow *GameWindowManager::Win_Get_Window_List()
{
    return m_windowList;
}

void GameWindowManager::Register_Tab_List(std::list<GameWindow *> list)
{
    m_tabList.clear();
    m_tabList = list;
}

void GameWindowManager::Clear_Tab_List()
{
    m_tabList.clear();
}

GameWindow *GameWindowManager::Win_Get_Focus()
{
    return m_keyboardFocus;
}

void GameWindowManager::Win_Set_Grab_Window(GameWindow *window)
{
    m_grabWindow = window;
}

GameWindow *GameWindowManager::Win_Get_Grab_Window()
{
    return m_grabWindow;
}

int GameWindowManager::Win_Capture(GameWindow *window)
{
    if (m_mouseCaptor) {
        return -4;
    }

    m_mouseCaptor = window;
    return 0;
}

int GameWindowManager::Win_Release(GameWindow *window)
{
    if (window == m_mouseCaptor) {
        m_mouseCaptor = nullptr;
    }
    return 0;
}

GameWindow *GameWindowManager::Get_Win_Capture()
{
    return m_mouseCaptor;
}

void GameWindowManager::Win_Draw_Image(Image *image, int start_x, int start_y, int end_x, int end_y, int color)
{
    g_theDisplay->Draw_Image(image, start_x, start_y, end_x, end_y, color, Display::DRAWIMAGE_ADDITIVE);
}

void GameWindowManager::Win_Fill_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Open_Rect(start_x, start_y, end_x - start_x, end_y - start_y, width, color);
}

void GameWindowManager::Win_Open_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Open_Rect(start_x, start_y, end_x - start_x, end_y - start_y, width, color);
}

void GameWindowManager::Win_Draw_Line(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Line(start_x, start_y, end_x, end_y, width, color);
}

int GameWindowManager::Win_Make_Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
    return Make_Color(red, green, blue, alpha);
}

int GameWindowManager::Win_Font_Height(GameFont *font)
{
    return font->m_height;
}

int GameWindowManager::Win_Is_Digit(int c)
{
    return iswdigit(c);
}

int GameWindowManager::Win_Is_Ascii(int c)
{
    return iswascii(c);
}

int GameWindowManager::Win_Is_Alpha_Numeric(int c)
{
    return iswalnum(c);
}

void GameWindowManager::Process_Destroy_List()
{
#if 0
    GameWindow window = m_destroyList;
    m_destroyList = nullptr;

    while (window != nullptr) {
        GameWindow *next = window->m_next;

        if (m_mouseCaptor == window) {
            Win_Release(window);
        }

        if (m_keyboardFocus == window) {
            Win_Set_Focus(0);
        }

        if (m_modalHead != nullptr) {
            if (window == m_modalHead->window) {
                Win_Unset_Modal(m_modalHead->window);
            }
        }

        if (m_currMouseRgn == window) {
            m_currMouseRgn = nullptr;
        }

        if (m_grabWindow == window) {
            m_grabWindow = nullptr;
        }

        winSendSystemMsg(window, 2, 0, 0);

        delete window;

        window = next;
    }
#endif
}