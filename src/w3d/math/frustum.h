#pragma once

#include "plane.h"
#include "vector3.h"

class FrustumClass
{
public:
    void Init(const Matrix3D &camera, const Vector2 &viewport_min, const Vector2 &viewport_max, float znear, float zfar);

    const Vector3 &Get_Bound_Min(void) const { return m_boundMin; }
    const Vector3 &Get_Bound_Max(void) const { return m_boundMax; }

public:
    Matrix3D m_cameraTransform;
    PlaneClass m_planes[6];
    Vector3 m_corners[8];
    Vector3 m_boundMin;
    Vector3 m_boundMax;
};