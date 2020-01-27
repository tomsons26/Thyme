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
#include "matrix3d.h"
#include "vector3.h"

// axis aligned box

//NOTE currently incomplete, add to as functions are discovered
class AABoxClass
{
public:
    AABoxClass() {}

    AABoxClass(const Vector3 &center, const Vector3 &extent) : m_center(center), m_extent(extent) {}

    void Init(const Vector3 &center, const Vector3 &extent)
    {
        m_center = center;
        m_extent = extent;
    }

    void Transform(const Matrix3D &tm);

    Vector3 m_center; // world space center
    Vector3 m_extent; // size of the box in the three directions
};

inline void AABoxClass::Transform(const Matrix3D &tm)
{
    Vector3 old_center = m_center;
    Vector3 old_extent = m_extent;
    tm.Transform_Center_Extent_AABox(old_center, old_extent, &m_center, &m_extent);
}
