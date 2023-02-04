/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Pathfinder Path and PathNode
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
#include "coord.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"

// confirmed
struct ClosestPointOnPathInfo
{
    float m_distance;
    Coord3D m_pos;
    PathfindLayerEnum m_layer;
};

class PathNode : public MemoryPoolObject
{
    IMPLEMENT_POOL(PathNode)

public:
    PathNode();
    virtual ~PathNode() override {}

    void Set_Next_Optimized(PathNode *node);

    PathNode *Get_Next_Optimized(Coord2D *pos = nullptr, float *length = nullptr);
    const PathNode *Get_Next_Optimized(Coord2D *pos, float *length) const;

    PathNode *Prepend_To_List(PathNode *node);

    PathNode *Append_To_List(PathNode *node);
    PathNode *Append(PathNode *node);

    const Coord3D *Compute_Direction_Vector();

    void Set_Position(const Coord3D *pos) { m_pos = *pos; }
    Coord3D *Get_Position() { return &m_pos; }
    const Coord3D *Get_Position() const { return &m_pos; }

    PathNode *Get_Next() { return m_next; }
    PathNode *Get_Prev() { return m_prev; }

    const PathNode *Get_Next() const { return m_next; }
    const PathNode *Get_Prev() const { return m_prev; }

    void Set_Layer(PathfindLayerEnum layer) { m_layer = layer; }
    PathfindLayerEnum Get_Layer() { return m_layer; }

    void Set_UnkBool(bool state) { m_unkBool = state; }
    bool Get_UnkBool() { return m_unkBool; }

private:
    int m_optimizedLink; // not 100% confirmed
    PathNode *m_nextOpti; // confirmed
    PathNode *m_next; // confirmed
    PathNode *m_prev; // confirmed
    Coord3D m_pos; // confirmed
    PathfindLayerEnum m_layer; // not 100% confirmed
    bool m_unkBool; // not 100% confirmed
    float m_optimizedLength; // not 100% confirmed
    Coord2D m_optimizedPos; // not 100% confirmed
};

inline PathNode *PathNode::Get_Next_Optimized(Coord2D *pos, float *length)
{
    if (pos != nullptr) {
        pos->x = m_optimizedPos.x;
        pos->y = m_optimizedPos.y;
    }
    if (length != nullptr) {
        *length = m_optimizedLength;
    }

    return m_nextOpti;
}


inline const PathNode *PathNode::Get_Next_Optimized(Coord2D *pos, float *length) const
{
    if (pos != nullptr) {
        pos->x = m_optimizedPos.x;
        pos->y = m_optimizedPos.y;
    }
    if (length != nullptr) {
        *length = m_optimizedLength;
    }

    return m_nextOpti;
}

class Path : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Path);

    Path();

protected:
    virtual ~Path() override;


public:
    virtual void CRC_Snapshot(Xfer *xfer) override{}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Get_Closest_Point_Pos(Coord3D *pos) const { *pos = m_closestPoint.m_pos; }

    void Prepend_Node(const Coord3D *pos, PathfindLayerEnum layer);
    void Append_Node(const Coord3D *pos, PathfindLayerEnum layer);
    void Update_Last_Node(const Coord3D *pos);

    const PathNode *Set_As_Optimized() { m_isOptimized; }

private:
    PathNode *m_pathHead; // confirmed
    PathNode *m_pathTail; // confirmed
    bool m_isOptimized; // confirmed
    bool m_blockedByAlly; // confirmed
    bool m_unk2; // not 100% confirmed
    int m_unk3; // not 100% confirmed
    Coord3D m_unk4; // not 100% confirmed
    ClosestPointOnPathInfo m_closestPoint; // not 100% confirmed
    PathNode *m_unkNode; // not 100% confirmed
};
