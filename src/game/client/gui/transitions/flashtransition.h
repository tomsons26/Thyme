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
#include "transition.h"

class FlashTransition : public Transition
{
public:
    FlashTransition();
    virtual ~FlashTransition();

    void Init(GameWindow *window) override;
    void Update(int) override;
    void Reverse() override;
    void Draw() override;
    void Skip() override;

protected:
    int m_xpos;
    int m_ypos;
    int m_width;
    int m_height;
    int int2;
};
