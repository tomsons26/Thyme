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
#include "sphere.h"
#include "vector3.h"

class RenderObjClass;

class IntersectionResultClass
{
public:
    RenderObjClass *m_intersectedRenderObject;
    unsigned short m_intersectedPolygon;

    Matrix3D m_modelMatrix;
    Vector3 m_modelLocation;
    Vector3 m_intersection;

    float m_range;
    float m_alpha;
    float m_beta;
    bool m_intersects;

    enum INTERSECTION_TYPE
    {
        NONE = 0,
        GENERIC,
        POLYGON
    } m_intersectionType;
};

// NOTE currently incomplete, add to as functions are discovered
class IntersectionClass
{
public:
    IntersectionClass();
    virtual ~IntersectionClass() {}

    void Append_Object_Array(int max_count, int &current_count, RenderObjClass **obj_array, RenderObjClass *obj);

    inline bool IntersectionClass::Intersect_Sphere_Quick(SphereClass &sphere, IntersectionResultClass *final);
    inline bool IntersectionClass::Intersect_Sphere(SphereClass &sphere, IntersectionResultClass *final);

public:
    Vector3 *m_rayLocation;
    Vector3 *m_rayDirection;
    Vector3 *m_intersectionNormal;
    float m_screenX;
    float m_screenY;
    bool m_interpolateNormal;
    bool m_convexTest;
    float m_maxDistance;
    IntersectionResultClass m_result;
};

inline bool IntersectionClass::Intersect_Sphere_Quick(SphereClass &sphere, IntersectionResultClass *final)
{
    Vector3 sphere_vector(sphere.Center - *m_rayLocation);

    final->m_alpha = Vector3::Dot_Product(sphere_vector, *m_rayDirection);

    final->m_beta = sphere.Radius * sphere.Radius
        - (Vector3::Dot_Product(sphere_vector, sphere_vector) - final->m_alpha * final->m_alpha);

    if (final->m_beta < 0.0f) {
        return final->m_intersects = false;
    }
    return final->m_intersects = true;
}

inline bool IntersectionClass::Intersect_Sphere(SphereClass &sphere, IntersectionResultClass *final)
{
    if (!Intersect_Sphere_Quick(sphere, final))
        return false;

    float d = sqrtf(final->m_beta);
    final->m_range = final->m_alpha - d;

    if (final->m_range > m_maxDistance)
        return false;

    final->m_intersection = *m_rayLocation + final->m_range * (*m_rayDirection);

    if (m_intersectionNormal != 0) {
        (*m_intersectionNormal) = final->m_intersection - sphere.Center;
    }
    return true;
}
