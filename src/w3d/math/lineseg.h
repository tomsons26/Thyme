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
#include "vector3.h"

class Matrix3D;

// NOTE currently incomplete, add to as functions are discovered
class LineSegClass
{
public:
    LineSegClass() {}

    void Set(const Vector3 &p0, const Vector3 &p1)
    {
        m_p0 = p0;
        m_p1 = p1;
        Recalculate();
    }

    void Compute_Point(float t, Vector3 *set) const { Vector3::Add(m_p0, t * m_dp, set); }

    const Vector3 &Get_P0() const { return m_p0; }
    const Vector3 &Get_P1() const { return m_p1; }
    const Vector3 &Get_DP() const { return m_dp; }
    const Vector3 &Get_Dir() const { return m_dir; }
    float Get_Length() const { return m_length; }

protected:
    void Recalculate()
    {
        m_dp = m_p1 - m_p0;
        m_dir = m_dp;
        m_dir.Normalize();
        m_length = m_dp.Length();
    }

public:
    Vector3 m_p0; // start point
    Vector3 m_p1; // end point
    Vector3 m_dp; // difference of the two points
    Vector3 m_dir; // normalized direction
    float m_length; // segment length
};
