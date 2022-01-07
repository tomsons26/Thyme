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
#include "colorspace.h"
#include "display.h"
#include "gamefont.h"
#include "gamewindowtransitions.h"
#include "globallanguage.h"
#include "image.h"
#include "windowlayout.h"

#ifndef GAME_DLL
GameWindowManager *g_theWindowManager;
#endif

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

GameWindowManager::~GameWindowManager()
{
    Win_Destroy_All();
    Free_Static_Strings();

    if (g_theTransitionHandler != nullptr) {
        delete g_theTransitionHandler;
    }
    // may need a stl call on tab list..
}

void GameWindowManager::Init()
{
    if (g_theTransitionHandler == nullptr) {
        g_theTransitionHandler = new GameWindowTransitionsHandler;
    }

    g_theTransitionHandler->Load();

    g_theTransitionHandler->Init();
}

void GameWindowManager::Reset()
{
    Win_Destroy_All();

    if (g_theTransitionHandler != nullptr) {
        g_theTransitionHandler->Reset();
    }
}

void GameWindowManager::Update()
{
    Process_Destroy_List();

    if (g_theTransitionHandler != nullptr) {
        g_theTransitionHandler->Update();
    }
}

void GameWindowManager::Link_Window(GameWindow *window)
{
    GameWindow *win = nullptr;

    for (GameWindow *i = m_windowList; i != nullptr; i = i->m_next) {

        for (ModalWindow *j = m_modalHead; j != nullptr; j = j->m_next) {
            if (j->m_window == i && j->m_window != window) {
                win = i;
            }
        }
    }
    if (win == nullptr) {
        window->m_prev = nullptr;
        window->m_next = m_windowList;

        if (m_windowList != nullptr) {
            m_windowList->m_prev = window;
        } else {
            m_windowTail = window;
        }

        m_windowList = window;
    } else {

        window->m_prev = win;
        window->m_next = win->m_next;
        win->m_next = window;

        if (window->m_next != nullptr) {
            window->m_next->m_prev = window;
        }
    }
}

void GameWindowManager::Insert_Window_Ahead_Of(GameWindow *window, GameWindow *ahead_of)
{
    if (window != nullptr) {
        if (ahead_of == nullptr) {
            Link_Window(window);
        } else {
            GameWindow *parent = ahead_of->Win_Get_Parent();

            if (parent == nullptr) {
                window->m_prev = ahead_of->m_prev;

                if (ahead_of->m_prev != nullptr) {
                    ahead_of->m_prev->m_next = window;
                } else {
                    m_windowList = window;
                }

                ahead_of->m_prev = window;
                window->m_next = ahead_of;

            } else {
                window->m_prev = ahead_of->m_prev;

                if (ahead_of->m_prev != nullptr) {
                    ahead_of->m_prev->m_next = window;
                } else {
                    parent->m_child = window;
                }

                ahead_of->m_prev = window;
                window->m_next = ahead_of;
                window->m_parent = parent;
            }
        }
    }
}

void GameWindowManager::Unlink_Window(GameWindow *window)
{
    if (window->m_next != nullptr) {
        window->m_next->m_prev = window->m_prev;
    } else {
        m_windowTail = window->m_prev;
    }

    if (window->m_prev != nullptr) {
        window->m_prev->m_next = window->m_next;
    } else {
        m_windowList = window->m_next;
    }
}

void GameWindowManager::Unlink_Child_Window(GameWindow *window)
{
    if (window->m_prev != nullptr) {
        window->m_prev->m_next = window->m_next;
        if (window->m_next) {
            window->m_next->m_prev = window->m_prev;
        }

    } else if (window->m_next != nullptr) {
        window->m_parent->m_child = window->m_next;
        window->m_next->m_prev = window->m_prev;
        window->m_next = 0;
    } else {
        window->m_parent->m_child = nullptr;
    }

    window->m_parent = nullptr;
}

bool GameWindowManager::Is_Enabled(GameWindow *window)
{
    if (window == nullptr) {
        return false;
    }

    if (!(window->m_status & WIN_STATUS_ENABLED)) {
        return false;
    }

    while (window->m_parent != nullptr) {
        window = window->m_parent;

        if (!(window->m_status & WIN_STATUS_ENABLED)) {
            return false;
        }
    }

    return true;
}

bool GameWindowManager::Is_Hidden(GameWindow *window)
{
    if (window == nullptr) {
        return true;
    }

    if (window->m_status & WIN_STATUS_HIDDEN) {
        return true;
    }

    while (window->m_parent != nullptr) {
        window = window->m_parent;

        if (window->m_status & WIN_STATUS_HIDDEN) {
            return true;
        }
    }

    return false;
}

void GameWindowManager::Add_Window_To_Parent(GameWindow *window, GameWindow *parent)
{
    if (parent != nullptr) {

        window->m_prev = nullptr;

        window->m_next = parent->m_child;

        if (parent->m_child != nullptr) {
            parent->m_child->m_prev = window;
        }

        parent->m_child = window;
        window->m_parent = parent;
    }
}

void GameWindowManager::Add_Window_To_Parent_At_End(GameWindow *window, GameWindow *parent)
{
    GameWindow *win;

    if (parent != nullptr) {
        window->m_prev = nullptr;
        window->m_next = nullptr;

        if (parent->m_child != nullptr) {
            for (win = parent->m_child; win->m_next != nullptr; win = win->m_next) {
                ;
            }

            win->m_next = window;
            window->m_prev = win;

        } else {
            parent->m_child = window;
        }

        window->m_parent = parent;
    }
}

void GameWindowManager::Window_Hiding(GameWindow *window)
{
    if (m_keyboardFocus == window) {
        m_keyboardFocus = nullptr;
    }

    if (m_modalHead != nullptr) {
        if (m_modalHead->m_window == window) {
            Win_Unset_Modal(window);
        }
    }

    if (m_mouseCaptor == window) {
        Win_Capture(nullptr);
    }

    for (GameWindow *i = window->Win_Get_Child(); i != nullptr; i = i->Win_Get_Next()) {
        Window_Hiding(i);
    }
}

void GameWindowManager::Hide_Windows_In_Range(GameWindow *base_window, int first, int last, bool state)
{
    for (int i = first; i <= last; ++i) {
        GameWindow *win = Win_Get_Window_From_Id(base_window, i);

        if (win != nullptr) {
            win->Win_Hide(state);
        }
    }
}

void GameWindowManager::Enable_Windows_In_Range(GameWindow *base_window, int first, int last, bool state)
{
    for (int i = first; i <= last; ++i) {
        GameWindow *win = Win_Get_Window_From_Id(base_window, i);

        if (win != nullptr) {
            win->Win_Enable(state);
        }
    }
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

GameWindow *GameWindowManager::Win_Get_Window_From_Id(GameWindow *window, int id)
{
    if (window == nullptr) {
        window = m_windowList;
    }

    while (window != nullptr) {

        if (window->Win_Get_Window_Id() == id) {
            return window;
        }

        if (window->m_child != nullptr) {
            GameWindow *wnd = Win_Get_Window_From_Id(window->m_child, id);

            if (wnd != nullptr) {
                return wnd;
            }
        }
        window = window->m_next;
    }

    return nullptr;
}

GameWindow *GameWindowManager::Win_Get_Window_List()
{
    return m_windowList;
}

WindowMsgHandledType GameWindowManager::Win_Send_System_Msg(GameWindow *window, unsigned msg, unsigned data1, unsigned data2)
{
    if (window == nullptr) {
        return MSG_IGNORED;
    }

    if (msg != 2 && window->m_status & 0x800) {
        return MSG_IGNORED;
    }

    return window->m_system(window, msg, data1, data2);
}

WindowMsgHandledType GameWindowManager::Win_Send_Input_Msg(GameWindow *window, unsigned msg, unsigned data1, unsigned data2)
{
    if (window == nullptr) {
        return MSG_IGNORED;
    }

    if (msg != 2 && window->m_status & 0x800) {
        return MSG_IGNORED;
    }

    return window->m_input(window, msg, data1, data2);
}

GameWindow *GameWindowManager::Win_Get_Focus()
{
    return m_keyboardFocus;
}

int GameWindowManager::Win_Set_Focus(GameWindow *window)
{
    unsigned key_res;
    unsigned win_res;

    key_res = 0;
    win_res = 0;

    if (window == nullptr || !(window->Win_Get_Status() & WIN_STATUS_NO_FOCUS)) {

        if (m_keyboardFocus != nullptr) {
            if (m_keyboardFocus != window) {
                Win_Send_System_Msg(m_keyboardFocus, GWM_INPUT_FOCUS, 0, (unsigned)&key_res);
            }
        }

        m_keyboardFocus = window;

        if (m_keyboardFocus != nullptr) {

            do {
                Win_Send_System_Msg(window, GWM_INPUT_FOCUS, 1, (unsigned)&win_res);

                if (win_res != 0) {
                    break;
                }

                window = window->Win_Get_Parent();
            } while (window != nullptr);
        }

        if (win_res == 0) {
            m_keyboardFocus = nullptr;
        }
    }

    return 0;
}

WinInputReturnCode GameWindowManager::Win_Process_Key(unsigned char key, unsigned char state)
{
    // TODO investigate why key 0 is ignored.....
    if (m_keyboardFocus != nullptr && key != 0) {

        GameWindow *window = m_keyboardFocus;

        while (Win_Send_Input_Msg(window, 0x15, key, state) == MSG_IGNORED) {
            window = window->Win_Get_Parent();

            if (window == nullptr) {
                return WIN_INPUT_NOT_USED;
            }
        }

        return WIN_INPUT_USED;
    }

    return WIN_INPUT_NOT_USED;
}

WinInputReturnCode GameWindowManager::Win_Process_Mouse_Event(GameWindowMessage msg, ICoord2D *mouse_pos, void *data)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<WinInputReturnCode, GameWindowManager, GameWindowMessage, ICoord2D *, void *>(
        PICK_ADDRESS(0xBAAAAAAAA, 0), this, msg, mouse_pos, data);
#else
    return WinInputReturnCode(0);
#endif
}

int GameWindowManager::Draw_Window(GameWindow *window)
{
    if (window == nullptr) {
        return -2;
    }

    if (window->m_status & WIN_STATUS_HIDDEN) {
        return 0;
    }

    if (!(window->m_status & WIN_STATUS_SEE_THRU)) {
        if (window->m_draw != nullptr) {
            window->m_draw(window, &window->m_instData);
        }
    }

    if (window->Win_Get_Style() & 0x20) {
        if (((window->m_status & WIN_STATUS_BORDER) != 0) == 1 && !(window->m_status & WIN_STATUS_SEE_THRU)) {
            window->Win_Draw_Border();
        }
    }

    GameWindow *child;
    for (child = window->m_child; child != nullptr && child->m_next != nullptr; child = child->m_next) {
        ;
    }

    while (child != nullptr) {
        Draw_Window(child);
        child = child->m_prev;
    }

    if (!(window->Win_Get_Style() & 0x20) && ((window->m_status & WIN_STATUS_BORDER) != 0) == 1
        && !(window->m_status & WIN_STATUS_SEE_THRU)) {
        window->Win_Draw_Border();
    }

    return 0;
}

void GameWindowManager::Dump_Window(GameWindow *window)
{
    if (window) {
        void *data = window->m_userData;
        WindowDrawFunc draw = window->m_draw;
        int id = window->Win_Get_Window_Id();
        captainslog_debug("ID: %d\tRedraw: 0x%08X\tUser Data: %d", id, draw, data);

        for (GameWindow *i = window->m_child; i != nullptr; i = i->m_next) {
            Dump_Window(i);
        }
    }
}

void GameWindowManager::Win_Repaint()
{
    GameWindow *prev;
    GameWindow *win;

    for (win = m_windowTail; win != nullptr; win = prev) {
        prev = win->m_prev;
        if (win->m_status & WIN_STATUS_BELOW) {
            Draw_Window(win);
        }
    }

    for (win = m_windowTail; win != nullptr; win = prev) {
        prev = win->m_prev;
        if (!(win->m_status & (WIN_STATUS_BELOW | WIN_STATUS_ABOVE))) {
            Draw_Window(win);
        }
    }

    for (win = m_windowTail; win != nullptr; win = prev) {
        prev = win->m_prev;
        if (win->m_status & WIN_STATUS_ABOVE) {
            Draw_Window(win);
        }
    }

    if (g_theTransitionHandler) {
        g_theTransitionHandler->Draw();
    }
}

GameWindow *GameWindowManager::Win_Create(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WindowCallbackFunc system,
    WinInstanceData *inst_data)
{
    GameWindow *window = Allocate_New_Window();

    if (window == nullptr) {

        captainslog_error("Could not allocate new window.");

        for (GameWindow *i = m_windowList; i != nullptr; i = i->m_next) {
            Dump_Window(i);
        }

        return nullptr;
    }

    if (parent != nullptr) {
        Add_Window_To_Parent(window, parent);
    } else {
        Link_Window(window);
    }

    window->m_status = status;
    window->m_size.x = width;
    window->m_size.y = height;
    window->m_region.lo.x = x;
    window->m_region.lo.y = y;
    window->m_region.hi.x = width + x;
    window->m_region.hi.y = height + y;

    window->Normalize_Window_Region();
    window->Win_Set_System_Func(system);

    Win_Send_System_Msg(window, 1, 0, 0);

    if (inst_data != nullptr) {
        window->Win_Set_Instance_Data(inst_data);
    }

    if (g_theGlobalLanguage != nullptr && g_theGlobalLanguage->Default_Window_Font().Name().Is_Not_Empty()) {
        const FontDesc &font = g_theGlobalLanguage->Default_Window_Font();

        window->Win_Set_Font(Win_Find_Font(font.Name(), font.Point_Size(), font.Bold()));
    } else {
        window->Win_Set_Font(Win_Find_Font("Times New Roman", 14, false));
    }

    return window;
}

int GameWindowManager::Win_Destroy(GameWindow *window)
{
    if (window == nullptr) {
        return -2;
    }

    captainslog_assert(window->Win_Get_Edit_Data() == nullptr, "edit data should NOT be present!");

    if (window->m_status & WIN_STATUS_DESTROYED) {
        return 0;
    }

    window->m_status |= WIN_STATUS_DESTROYED;

    // GameWindow_86F230(window);

    if (m_mouseCaptor == window) {
        Win_Release(window);
    }

    if (m_keyboardFocus == window) {
        Win_Set_Focus(nullptr);
    }

    if (m_modalHead != nullptr) {
        if (window == m_modalHead->m_window) {
            Win_Unset_Modal(m_modalHead->m_window);
        }
    }

    if (m_currMouseRgn == window) {
        m_currMouseRgn = nullptr;
    }

    if (m_grabWindow == window) {
        m_grabWindow = nullptr;
    }

    GameWindow *next;
    for (GameWindow *i = window->m_child; i != nullptr; i = next) {
        next = i->m_next;
        Win_Destroy(i);
    }

    if (window->m_parent == nullptr) {
        Unlink_Window(window);
    } else {
        Unlink_Child_Window(window);
    }

    window->m_prev = nullptr;
    window->m_next = m_destroyList;
    m_destroyList = window;

    if (window->m_layout != nullptr) {
        window->m_layout->Remove_Window(window);
    }

    return 0;
}

int GameWindowManager::Win_Destroy_All()
{
    GameWindow *i = m_windowList;

    for (GameWindow *j = m_windowList; j != nullptr; i = j) {
        j = i->m_next;
        Win_Destroy(i);
    }

    Process_Destroy_List();

    return 0;
}

int GameWindowManager::Win_Set_Modal(GameWindow *window)
{
    if (window == nullptr) {
        return -2;
    }

    if (window->m_parent != nullptr) {
        captainslog_debug("WinSetModal: Non Root window attempted to go modal.");
        return -3;
    }

    ModalWindow *modal = new ModalWindow;

    if (modal == nullptr) {
        captainslog_debug("WinSetModal: Unable to allocate space for Modal Entry.");
        return -1;
    }

    modal->m_window = window;
    modal->m_next = m_modalHead;
    m_modalHead = modal;

    return 0;
}

int GameWindowManager::Win_Unset_Modal(GameWindow *window)
{

    if (window == nullptr) {
        return -2;
    }

    if (m_modalHead == nullptr || m_modalHead->m_window != window) {
        captainslog_debug("Invalid window attempting to unset modal (%d).", window->Win_Get_Window_Id());
        return -1;
    }

    ModalWindow *modal = m_modalHead->m_next;

    m_modalHead->Delete_Instance();

    m_modalHead = modal;

    return 0;
}

GameWindow *GameWindowManager::Win_Get_Grab_Window()
{
    return m_grabWindow;
}

void GameWindowManager::Win_Set_Grab_Window(GameWindow *window)
{
    m_grabWindow = window;
}

void GameWindowManager::Win_Set_Lone_Window(GameWindow *window)
{
    if (m_loneWindow != window) {
        if (m_loneWindow != nullptr) {
            g_theWindowManager->Win_Send_System_Msg(m_loneWindow, 0x4005, 0, 0);
        }

        m_loneWindow = window;
    }
}

GameWindow *GameWindowManager::Go_Go_Message_Box(int x,
    int y,
    int width,
    int height,
    unsigned short flags,
    Utf16String title,
    Utf16String body,
    WindowMsgBoxCallbackFunc yes_callback,
    WindowMsgBoxCallbackFunc no_callback,
    WindowMsgBoxCallbackFunc ok_callback,
    WindowMsgBoxCallbackFunc cancel_callback)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        int,
        int,
        int,
        int,
        unsigned short,
        Utf16String,
        Utf16String,
        WindowMsgBoxCallbackFunc,
        WindowMsgBoxCallbackFunc,
        WindowMsgBoxCallbackFunc,
        WindowMsgBoxCallbackFunc>(PICK_ADDRESS(0xBAAAAAD, 0),
        this,
        x,
        y,
        width,
        height,
        flags,
        title,
        body,
        yes_callback,
        no_callback,
        ok_callback,
        cancel_callback);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Message_Box(int x,
    int y,
    int width,
    int height,
    unsigned short flags,
    Utf16String title,
    Utf16String body,
    WindowMsgBoxCallbackFunc yes_callback,
    WindowMsgBoxCallbackFunc no_callback,
    WindowMsgBoxCallbackFunc ok_callback,
    WindowMsgBoxCallbackFunc cancel_callback,
    bool use_other)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        int,
        int,
        int,
        int,
        unsigned short,
        Utf16String,
        Utf16String,
        WindowMsgBoxCallbackFunc,
        WindowMsgBoxCallbackFunc,
        WindowMsgBoxCallbackFunc,
        WindowMsgBoxCallbackFunc,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0),
        this,
        x,
        y,
        width,
        height,
        flags,
        title,
        body,
        yes_callback,
        no_callback,
        ok_callback,
        cancel_callback,
        use_other);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Push_Button(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Checkbox(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Radio_Button(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _RadioButtonData *data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        _RadioButtonData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Tab_Control(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _TabControlData *data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        _TabControlData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_List_Box(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _ListboxData *data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        _ListboxData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Slider(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _SliderData *data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        _SliderData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Combo_Box(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _ComboBoxData *data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        _ComboBoxData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Progress_Bar(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Static_Text(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _TextData *data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        _TextData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, data, font, assign_visuals);
#else
    return nullptr;
#endif
}

GameWindow *GameWindowManager::Go_Go_Gadget_Text_Entry(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _EntryData *data,
    GameFont *font,
    bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *,
        GameWindowManager,
        GameWindow *,
        unsigned int,
        int,
        int,
        int,
        int,
        WinInstanceData *,
        _EntryData *,
        GameFont *,
        bool>(PICK_ADDRESS(0xBAAAAAD, 0), this, parent, status, x, y, width, height, inst_data, data, font, assign_visuals);
#else
    return nullptr;
#endif
}

void GameWindowManager::Assign_Default_Gadget_Look(GameWindow *gadget, GameFont *font, bool assign_visuals)
{
    // TODO
#ifdef GAME_DLL
    Call_Method<void, GameWindowManager, GameWindow *, GameFont *, bool>(
        PICK_ADDRESS(0xBAAAAAD, 0), this, gadget, font, assign_visuals);
#endif
}

Utf16String GameWindowManager::Win_Text_Label_To_Text(Utf8String label)
{
    if (label.Is_Empty()) {
        return Utf16String::s_emptyString;
    }

    Utf16String text;

    text.Translate(label);

    return text;
}

GameWindow *GameWindowManager::Get_Win_Under_Cursor(int x, int y, bool ignore_enabled)
{
    GameWindow *win = nullptr;

    if (m_mouseCaptor != nullptr) {
        return m_mouseCaptor->Win_Point_In_Child(x, y, ignore_enabled, false);
    }

    if (m_grabWindow != nullptr) {
        return m_grabWindow->Win_Point_In_Child(x, y, ignore_enabled, false);
    }

    if (m_modalHead != nullptr && m_modalHead->m_window != nullptr) {
        return m_modalHead->m_window->Win_Point_In_Child(x, y, ignore_enabled, false);
    }

    for (win = m_windowList; win != nullptr; win = win->m_next) {

        if (win->m_status & WIN_STATUS_ABOVE && !(win->m_status & WIN_STATUS_HIDDEN) && x >= win->m_region.lo.x
            && x <= win->m_region.hi.x && y >= win->m_region.lo.y && y <= win->m_region.hi.y
            && (win->m_status & WIN_STATUS_ENABLED || ignore_enabled)) {
            win = win->Win_Point_In_Child(x, y, ignore_enabled, false);
            break;
        }
    }

    if (win == nullptr) {
        for (win = m_windowList; win != nullptr; win = win->m_next) {

            if (!(win->m_status & (WIN_STATUS_BELOW | WIN_STATUS_ABOVE | WIN_STATUS_HIDDEN)) && x >= win->m_region.lo.x
                && x <= win->m_region.hi.x && y >= win->m_region.lo.y && y <= win->m_region.hi.y
                && (win->m_status & WIN_STATUS_ENABLED || ignore_enabled)) {
                win = win->Win_Point_In_Child(x, y, ignore_enabled, false);
                break;
            }
        }
    }

    if (win == nullptr) {
        for (win = m_windowList; win != nullptr; win = win->m_next) {

            if (win->m_status & WIN_STATUS_BELOW && !(win->m_status & WIN_STATUS_HIDDEN) && x >= win->m_region.lo.x
                && x <= win->m_region.hi.x && y >= win->m_region.lo.y && y <= win->m_region.hi.y
                && (win->m_status & WIN_STATUS_ENABLED || ignore_enabled)) {
                win = win->Win_Point_In_Child(x, y, ignore_enabled, false);
                break;
            }
        }
    }

    if (win != nullptr) {
        if (win->m_status & WIN_STATUS_NO_INPUT) {
            win = nullptr;
        } else if (ignore_enabled && !(win->m_status & WIN_STATUS_ENABLED)) {
            win = nullptr;
        }
    }

    return win;
}

void GameWindowManager::Win_Next_Tab(GameWindow *window)
{
    // TODO
#ifdef GAME_DLL
    Call_Method<void, GameWindowManager, GameWindow *>(PICK_ADDRESS(0xBAAAAAD, 0), this, window);
#endif
}

void GameWindowManager::Win_Prev_Tab(GameWindow *window)
{
    // TODO
#ifdef GAME_DLL
    Call_Method<void, GameWindowManager, GameWindow *>(PICK_ADDRESS(0xBAAAAAD, 0), this, window);
#endif
}

void GameWindowManager::Register_Tab_List(std::list<GameWindow *> *list)
{
    // TODO
#ifdef GAME_DLL
    Call_Method<void, GameWindowManager, std::list<GameWindow *> *>(PICK_ADDRESS(0xBAAAAAD, 0), this, list);
#endif
}

void GameWindowManager::Clear_Tab_List()
{
    // TODO
#ifdef GAME_DLL
    Call_Method<void, GameWindowManager>(PICK_ADDRESS(0xBAAAAAD, 0), this);
#endif
}

WindowDrawFunc GameWindowManager::Get_Default_Draw()
{ // TODO
#ifdef GAME_DLL
    return Call_Method<WindowDrawFunc, GameWindowManager>(PICK_ADDRESS(0xBAAAAAD, 0), this);
#endif
}

WindowCallbackFunc GameWindowManager::Get_Default_System()
{ // TODO
#ifdef GAME_DLL
    return Call_Method<WindowCallbackFunc, GameWindowManager>(PICK_ADDRESS(0xBAAAAAD, 0), this);
#endif
}

WindowCallbackFunc GameWindowManager::Get_Default_Input()
{ // TODO
#ifdef GAME_DLL
    return Call_Method<WindowCallbackFunc, GameWindowManager>(PICK_ADDRESS(0xBAAAAAD, 0), this);
#endif
}

WindowTooltipFunc GameWindowManager::Get_Default_Tooltip()
{ // TODO
#ifdef GAME_DLL
    return Call_Method<WindowTooltipFunc, GameWindowManager>(PICK_ADDRESS(0xBAAAAAD, 0), this);
#endif
}

void GameWindowManager::Win_Draw_Image(Image *image, int start_x, int start_y, int end_x, int end_y, int color)
{
    g_theDisplay->Draw_Image(image, start_x, start_y, end_x, end_y, color, Display::DRAWIMAGE_ADDITIVE);
}

void GameWindowManager::Win_Draw_Fill_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Open_Rect(start_x, start_y, end_x - start_x, end_y - start_y, width, color);
}

void GameWindowManager::Win_Draw_Open_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Open_Rect(start_x, start_y, end_x - start_x, end_y - start_y, width, color);
}

void GameWindowManager::Win_Draw_Line(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Line(start_x, start_y, end_x, end_y, width, color);
}

const Image *GameWindowManager::Win_Find_Image(const char *name)
{
    if (g_theMappedImageCollection) {
        return g_theMappedImageCollection->Find_Image_By_Name(name);
    }

    return nullptr;
}

int GameWindowManager::Win_Make_Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
    return Make_Color(red, green, blue, alpha);
}

void GameWindowManager::Win_Format_Text(GameFont *font, Utf16String text, int color, int x, int y, int width, int height)
{
    // TODO
#ifdef GAME_DLL
    Call_Method<void, GameWindowManager, GameFont *, Utf16String, int, int, int, int, int>(
        PICK_ADDRESS(0xBAAAAAAAA, 0), this, font, text, color, x, y, width, height);
#endif
}

void GameWindowManager::Win_Get_Text_Size(GameFont *font, Utf16String text, int *width, int *height, int max_width)
{
    // TODO
#ifdef GAME_DLL
    Call_Method<void, GameWindowManager, GameFont *, Utf16String, int *, int *, int>(
        PICK_ADDRESS(0xBAAAAAAAA, 0), this, font, text, width, height, max_width);
#endif
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

GameFont *GameWindowManager::Win_Find_Font(Utf8String font_name, int point_size, bool bold)
{
    if (g_theFontLibrary != nullptr) {
        return g_theFontLibrary->Get_Font(font_name, point_size, bold);
    }

    return nullptr;
}

void GameWindowManager::Process_Destroy_List()
{
    GameWindow *window = m_destroyList;

    m_destroyList = nullptr;

    while (window != nullptr) {
        GameWindow *next = window->m_next;

        if (m_mouseCaptor == window) {
            Win_Release(window);
        }

        if (m_keyboardFocus == window) {
            Win_Set_Focus(nullptr);
        }

        if (m_modalHead != nullptr) {
            if (window == m_modalHead->m_window) {
                Win_Unset_Modal(m_modalHead->m_window);
            }
        }

        if (m_currMouseRgn == window) {
            m_currMouseRgn = nullptr;
        }

        if (m_grabWindow == window) {
            m_grabWindow = nullptr;
        }

        Win_Send_System_Msg(window, 2, 0, 0);

        window->Delete_Instance();

        window = next;
    }
}
