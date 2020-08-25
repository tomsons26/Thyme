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
#include "debugdisplay.h"
#include <stdio.h>
#include <stdarg.h>

//fixup me
void DebugDisplay::Printf(char *format, ...)
{
    static char _text[5120];

    va_list va;
    va_start(va, format);
    int length = vsprintf(_text, format, va);
    if (length >= 0) {
        // if (length >= 5120 && !byte_E27E1C) {
        //    TheCurrentAllowCrashPtr = &byte_E27E1C;
        //    DebugCrash("text overflow in DebugDisplay::printf() - string too long");
        //    TheCurrentAllowCrashPtr = 0;
        //}

        char* v5 = _text;
        char *v2 = _text;
        int v4 = 0;
        while (1) {
            char v3 = *v5++;
            if (v3 == 0) {
                break;
            }
            if (v3 == '\n') {
                if (v4 > 0) {
                    *(v5 - 1) = 0;
                    Draw_Text(m_xPos + m_rightMargin, m_yPos, v2);
                    v4 = 0;
                }
                v2 = v5;
                ++m_yPos;
                m_xPos = 0;
            } else {
                ++v4;
            }
        }
        if (v4 > 0) {
            Draw_Text(m_xPos + m_rightMargin, m_yPos, v2);
            m_xPos += v4;
        }
    }
}