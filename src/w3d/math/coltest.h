#pragma once
#include "always.h"
#include "castres.h"
#include "lineseg.h"

// NOTE currently incomplete, add to as functions are discovered
class CollisionTestClass
{
public:
    inline CollisionTestClass(CastResultStruct *res, int collision_type);

public:
    CastResultStruct *m_result;
    int m_collisionType;
    RenderObjClass *m_collidedRenderObj;
};

inline CollisionTestClass::CollisionTestClass(CastResultStruct *res, int collision_type) :
    m_result(res), m_collisionType(collision_type), m_collidedRenderObj(nullptr)
{
}

// NOTE currently incomplete, add to as functions are discovered
class RayCollisionTestClass : public CollisionTestClass
{
public:
    RayCollisionTestClass(const LineSegClass &ray, CastResultStruct *res, int collision_type, bool bool1, bool bool2);

public:
    LineSegClass m_ray;
    bool m_bool1;
    bool m_bool2;
    bool m_bool3;
};

inline RayCollisionTestClass::RayCollisionTestClass(
    const LineSegClass &ray, CastResultStruct *res, int collision_type, bool bool1, bool bool2) :
    CollisionTestClass(res, collision_type), m_ray(ray), m_bool1(bool1), m_bool2(bool2), m_bool3(false)
{
}
