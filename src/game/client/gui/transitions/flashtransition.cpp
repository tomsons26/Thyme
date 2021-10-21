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
#include "flashtransition.h"
 
FlashTransition::FlashTransition() : int2(-1)
{
    int1 = 8;
    char2 = 1;
}

FlashTransition::~FlashTransition()
{
    m_window = nullptr;
}

void FlashTransition::Reverse()
{
    m_finished = false;
    char2 = 0;
}

void FlashTransition::Draw()
{
    int color;

    switch (int2) {
        case 1:
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x64u);
            g_theDisplay->Draw_Open_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, 1.0, color);
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x21u);
            g_theDisplay->Draw_Fill_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, color);
            break;
        case 2:
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x96u);
            g_theDisplay->Draw_Open_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, 1.0, color);
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x42u);
            g_theDisplay->Draw_Fill_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, color);
            break;
        case 3:
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0xC8u);
            g_theDisplay->Draw_Open_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, 1.0, color);
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x63u);
            g_theDisplay->Draw_Fill_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, color);
            break;
        case 4:
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0xFAu);
            g_theDisplay->Draw_Open_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, 1.0, color);
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x4Bu);
            g_theDisplay->Draw_Fill_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, color);
            break;
        case 5:
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0xFAu);
            g_theDisplay->Draw_Open_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, 1.0, color);
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x32u);
            g_theDisplay->Draw_Fill_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, color);
            break;
        case 6:
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0xFAu);
            g_theDisplay->Draw_Open_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, 1.0, color);
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0x19u);
            g_theDisplay->Draw_Fill_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, color);
            break;
        case 7:
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0xFAu);
            g_theDisplay->Draw_Open_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, 1.0, color);
            color = Make_Color(0xFFu, 0xFFu, 0xFFu, 0xAu);
            g_theDisplay->Draw_Fill_Rect(m_xpos + 1, m_ypos + 1, m_width - 2, m_height, color);
            break;
        default:
            break;
    }
}

void FlashTransition::Skip()
{
    Update(8);
}
