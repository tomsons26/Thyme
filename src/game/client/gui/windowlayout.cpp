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
#include "windowlayout.h"
#include "gamewindow.h"
#include "gamewindowmanager.h"
#include <captainslog.h>

WindowLayout::WindowLayout() :
    m_filenameString("EmptyLayout"),
    m_windowList(nullptr),
    m_windowTail(nullptr),
    m_windowCount(0),
    m_hidden(false),
    m_initFunc(nullptr),
    m_updateFunc(nullptr),
    m_shutdownFunc(nullptr)
{
}

WindowLayout::~WindowLayout()
{
    captainslog_dbgassert(m_windowList == nullptr, "Window layout being destroyed still has window references");
    captainslog_dbgassert(m_windowTail == nullptr, "Window layout being destroyed still has window references");
}

void WindowLayout::Hide(bool hide)
{
    for (GameWindow *i = m_windowList; i != nullptr; i = i->Win_Get_Next_In_Layout()) {
        i->Win_Hide(hide);
    }

    m_hidden = hide;
}

void WindowLayout::Add_Window(GameWindow *window)
{
    if (Find_Window(window) == nullptr) {

        captainslog_dbgassert(window->Win_Get_Next_In_Layout() == nullptr, "Next_In_Layout should be NULL before adding");
        captainslog_dbgassert(window->Win_Get_Prev_In_Layout() == nullptr, "Prev_In_Layout should be NULL before adding");

        window->Win_Set_Prev_In_Layout(nullptr);
        window->Win_Set_Next_In_Layout(m_windowList);

        if (m_windowList != nullptr) {
            m_windowList->Win_Set_Prev_In_Layout(window);
        }

        m_windowList = window;

        window->Win_Set_Layout(this);

        if (m_windowTail == nullptr) {
            m_windowTail = window;
        }

        ++m_windowCount;
    }
}

void WindowLayout::Remove_Window(GameWindow *window)
{
    window = Find_Window(window);

    if (window != nullptr) {
        GameWindow *prev = window->Win_Get_Prev_In_Layout();
        GameWindow *next = window->Win_Get_Next_In_Layout();

        if (next != nullptr) {
            next->Win_Set_Prev_In_Layout(prev);
        }

        if (prev != nullptr) {
            prev->Win_Set_Next_In_Layout(next);
        } else {
            m_windowList = next;
        }

        window->Win_Set_Layout(nullptr);

        window->Win_Set_Next_In_Layout(nullptr);
        window->Win_Set_Prev_In_Layout(nullptr);

        if (m_windowTail == window) {
            m_windowTail = prev;
        }
        --m_windowCount;
    }
}

void WindowLayout::Destroy_Windows()
{
    while (true) {

        GameWindow *window = this->WindowLayout::Get_Window_List();
        if (window == nullptr) {
            break;
        }

        Remove_Window(window);

        g_theWindowManager->Win_Destroy(window);
    }
}

int WindowLayout::Load(Utf8String filename)
{
    if (filename.Is_Empty()) {
        return 0;
    }

    WindowLayoutInfo info;

    GameWindow *window = g_theWindowManager->Win_Create_From_Script(filename, &info);
    if (window == nullptr) {
        captainslog_dbgassert(window != nullptr, "Failed to load layout");

        captainslog_debug("WindowLayout::load - Unable to load layout file '%s'\n", filename.Str());

        return 0;
    }

    stl_8E56B0(&v17);
    for (v17._M_node = *stl_8793F0(&info.list, &v13);; stl_operator_pp_51(&v17)) {
        v4 = stl_879410(&v12);
        if (!_STL::_List_iterator_base::operator!=(&v17, v4)) {
            break;
        }
        v5 = stl_879510(&v17);

        Add_Window(*v5);
    }

    m_filenameString = filename;

    Set_Init(info.m_initFunc);
    Set_Update(info.m_updateFunc);
    Set_Shutdown(info.m_shutdownFunc);

    return 1;
}

void WindowLayout::Bring_Forward()
{
    int count = m_windowCount;
    GameWindow *cur = m_windowTail;

    while (count) {

        captainslog_dbgassert(cur != nullptr, "Must have window: m_windowCount is off");

        GameWindow *prev = cur->Win_Get_Prev_In_Layout();

        cur->Win_Bring_To_Top();

        --count;

        cur = prev;
    }
}

GameWindow *WindowLayout::Find_Window(GameWindow *window)
{
    for (GameWindow *i = m_windowList; i != nullptr; i = i->Win_Get_Next_In_Layout()) {

        if (i == window) {
            return i;
        }
    }

    return nullptr;
}