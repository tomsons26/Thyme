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

class Transition
{
public:
    Transition();

    virtual ~Transition() {}

    virtual void Init(GameWindow *window) = 0;
    virtual void Update(int) = 0;
    virtual void Reverse() = 0;
    virtual void Draw() = 0;
    virtual void Skip() = 0;

    bool Is_Finished() { return m_finished; }

    int Get_Int1() { return int1; }

protected:
    int int1;
    bool m_finished;
    char char2;
    GameWindow *m_window;
};

inline Transition::Transition() : int1(0), m_finished(0), char2(0), m_window(nullptr) {}
