/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Stores results of a ray or volume cast
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
#include "vector3.h"

// NOTE currently incomplete, add to as functions are discovered
struct CastResultStruct
{
    CastResultStruct() { Reset(); }

    void Reset()
    {
        m_startBad = false;
        m_fraction = 1.0f;
        m_normal.Set(0.0f, 0.0f, 0.0f);
        m_surfaceType = 0;
        m_computeContactPoint = false;
        m_contactPoint.Set(0.0f, 0.0f, 0.0f);
    }

    bool m_startBad;
    float m_fraction;
    Vector3 m_normal;
    uint32_t m_surfaceType;
    bool m_computeContactPoint;
    Vector3 m_contactPoint;
};
