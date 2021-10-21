/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Classes for handling WND UI system transitions.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamewindowtransitions.h"
#include "color.h"
#include "transition.h"
#include "flashtransition.h"
#include "display.h"
#include <new>

Transition *Get_Transition_For_Style(WindowTransitionStyle style)
{
    Transition *t = nullptr;
    switch (style) {

        case TRANSITION_FLASH:
            t = new FlashTransition;
            break;
        case TRANSITION_BUTTON_FLASH:
            break;
        case TRANSITION_FADE:
            break;
        case TRANSITION_SCALE_UP:
            break;
        case TRANSITION_MAIN_MENU_SCALE_UP:
            break;
        case TRANSITION_TYPE_TEXT:
            break;
        case TRANSITION_SCREEN_FADE:
            break;
        case TRANSITION_COUNT_UP:
            break;
        case TRANSITION_FULL_FADE:
            break;
        case TRANSITION_TEXT_ON_FRAME:
            break;
        case TRANSITION_MAIN_MENU_MEDIUM_SCALE_UP:
            break;
        case TRANSITION_MAIN_MENU_SMALL_SCALE_DOWN:
            break;
        case TRANSITION_CONTROL_BAR_ARROW:
            break;
        case TRANSITION_SCORE_SCALE_UP:
            break;
        case TRANSITION_REVERSE_SOUND:
            break;
        default:
            captainslog_error("An invalid style was passed in. Style = %d", style);
            break;
    }

    return t;
}

bool TransitionWindow::Init()
{
    m_nameKey = g_theNameKeyGenerator->Name_To_Key(m_windowName);
    m_window = 0; // g_theWindowManager->Win_Get_Window_From_Id(0, m_nameKey);

    int1 = m_frameDelay;

    if (m_transition) {
        delete m_transition;
    }

    m_transition = Get_Transition_For_Style(m_style);

    m_transition->Init(m_window);

    return true;
}

void TransitionWindow::Update(int a1)
{
    if (a1 >= int1 && a1 <= m_transition->Get_Int1() + int1) {
        if (m_transition) {
            m_transition->Update(a1 - int1);
        }
    }
}

void TransitionWindow::Draw()
{
    if (m_transition) {
        m_transition->Draw();
    }
}

void TransitionWindow::Skip()
{
    if (m_transition) {
        m_transition->Skip();
    }
}

void TransitionWindow::Reverse()
{
    if (m_transition) {
        m_transition->Reverse();
    }
}

bool TransitionWindow::Is_Finished()
{
    if (m_transition) {
        return m_transition->Is_Finished();
    }

    return true;
}

TransitionGroup::TransitionGroup() : char1(0), int1(0), int2(0) {}

TransitionGroup::~TransitionGroup() {}

void TransitionGroup::Init()
{
    int1 = 0;
    int2 = 0;

    for (auto it : m_windowList) {
        it->Init();
    }
}

void TransitionGroup::Update()
{
    int2 += int1;

    for (auto it : m_windowList) {
        it->Update(int2);
    }
}

void TransitionGroup::Draw()
{
    for (auto it : m_windowList) {
        it->Draw();
    }
}

void TransitionGroup::Skip()
{
    for (auto it : m_windowList) {
        it->Skip();
    }
}

bool TransitionGroup::Is_Finished()
{
    for (auto it : m_windowList) {
        if (!it->Is_Finished()) {
            return false;
        }
    }
    return true;
}

GameWindowTransitionsHandler::GameWindowTransitionsHandler() :
    m_group1(nullptr), m_group2(nullptr), m_group3(nullptr), m_group4(nullptr)
{
}

void GameWindowTransitionsHandler::Init()
{
    m_group1 = nullptr;
    m_group2 = nullptr;
    m_group3 = nullptr;
    m_group4 = nullptr;
}

void GameWindowTransitionsHandler::Reset()
{
    m_group1 = nullptr;
    m_group2 = nullptr;
    m_group3 = nullptr;
    m_group4 = nullptr;
}

void GameWindowTransitionsHandler::Update()
{
    if (m_group3 == m_group1) {
        m_group4 = nullptr;
    } else {
        m_group4 = m_group3;
    }

    m_group3 = m_group1;
    if (m_group1) {
        if (!m_group1->Is_Finished()) {
            m_group1->Update();
        }
    }
    if (m_group1) {
        if (m_group1->Is_Finished()) {
            if (m_group1->Get_Char1()) {
                m_group1 = nullptr;
            }
        }
    }
    if (m_group1) {
        if (m_group2) {
            if (m_group1->Is_Finished()) {
                m_group1 = m_group2;
                m_group2 = nullptr;
            }
        }
    }
    if (!m_group1) {
        if (m_group2) {
            m_group1 = m_group2;
            m_group2 = nullptr;
        }
    }
    if (m_group1) {
        if (m_group1->Is_Finished()) {
            if (m_group1->Is_Reversed()) {
                m_group1 = nullptr;
            }
        }
    }
}

void GameWindowTransitionsHandler::Draw()
{
    if (m_group3) {
        m_group3->Draw();
    }
    if (m_group4) {
        m_group4->Draw();
    }
}