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

#include "pathfinder_path.h"

#include "terrainlogic.h"

PathNode::PathNode() :
    m_optimizedLink(-1),
    m_nextOpti(nullptr),
    m_next(nullptr),
    m_prev(nullptr),
    m_pos(),
    m_layer(LAYER_INVALID),
    m_unkBool(false),
    m_optimizedLength(0),
    m_optimizedPos()
{
    m_pos.Zero();
}

void PathNode::Set_Next_Optimized(PathNode *node)
{
    m_nextOpti = node;

    if (node) {
        m_optimizedPos.x = node->Get_Position()->x - Get_Position()->x;
        m_optimizedPos.y = node->Get_Position()->y - Get_Position()->y;

        m_optimizedLength = m_optimizedPos.Length();

        if (m_optimizedLength == 0.0) {
            m_optimizedLength = 0.0099999998;
        }

        m_optimizedPos.x = m_optimizedPos.x / m_optimizedLength;
        m_optimizedPos.y = m_optimizedPos.y / m_optimizedLength;
    } else {
        m_optimizedLength = 0.0;
    }
}

PathNode *PathNode::Prepend_To_List(PathNode *node)
{
    m_next = node;
    if (node != nullptr) {
        node->m_prev = this;
    }
    m_prev = nullptr;

    return this;
}

PathNode *PathNode::Append_To_List(PathNode *node)
{
    PathNode *n;

    if (node == nullptr) {
        m_next = 0;
        m_prev = 0;
        return this;
    }
    for (n = node; n->m_next; n = n->m_next) {
        ;
    }
    n->m_next = this;
    m_prev = n;
    m_next = nullptr;

    return node;
}

PathNode *PathNode::Append(PathNode *node)
{
    node->m_next = m_next;
    node->m_prev = this;
    if (node->m_next != nullptr) {
        node->m_next->m_prev = node;
    }

    m_next = node;
    return this;
}

const Coord3D *PathNode::Compute_Direction_Vector()
{
    static Coord3D PathDirVector;

    if (m_next == nullptr) {
        if (m_prev != nullptr) {
            return m_prev->Compute_Direction_Vector();
        }

        PathDirVector.x = 0.0;
        PathDirVector.y = 0.0;
        PathDirVector.z = 0.0;
    } else {
        PathDirVector.x = m_next->m_pos.x - m_pos.x;
        PathDirVector.y = m_next->m_pos.y - m_pos.y;
        PathDirVector.z = m_next->m_pos.z - m_pos.z;
    }
    return &PathDirVector;
}

Path::Path() :
    m_pathHead(nullptr),
    m_pathTail(nullptr),
    m_isOptimized(false),
    m_blockedByAlly(false),
    m_unk2(false),
    m_unk3(0),
    m_unk4(),
    m_closestPoint(),
    m_unkNode(nullptr)
{
    m_unk4.Zero();
    m_closestPoint.m_distance = 0.0;
    m_closestPoint.m_layer = LAYER_GROUND;
    m_closestPoint.m_pos.Zero();
}

Path::~Path()
{
    PathNode *next;
    for (PathNode *node = m_pathHead; node != nullptr; node = next) {
        next = node->Get_Next();
        node->Delete_Instance();
    }
}

void Path::Prepend_Node(const Coord3D *pos, PathfindLayerEnum layer)
{
    PathNode *node = new PathNode;

    node->Set_Position(pos);
    node->Set_Layer(layer);
    m_pathHead = node->Prepend_To_List(m_pathHead);

    if (m_pathTail == nullptr) {
        m_pathTail = node;
    }

    m_isOptimized = false;
}

void Path::Append_Node(const Coord3D *pos, PathfindLayerEnum layer)
{

    if (m_isOptimized && m_pathTail != nullptr && pos->x == m_pathTail->Get_Position()->x
        && pos->y == m_pathTail->Get_Position()->y) {
        captainslog_warn("Warning - Path Seg length == 0, ignoring.");
    } else {
        PathNode *node = new PathNode;

        node->Set_Position(pos);
        node->Set_Layer(layer);
        m_pathHead = node->Append_To_List(m_pathHead);
        if (m_isOptimized) {
            if (m_pathTail != nullptr) {
                m_pathTail->Set_Next_Optimized(node);
            }
        }
        m_pathTail = node;
    }
}

void Path::Update_Last_Node(const Coord3D *pos)
{
    PathfindLayerEnum layer = g_theTerrainLogic->Get_Layer_For_Destination(pos);

    if (m_pathTail != nullptr) {
        m_pathTail->Set_Position(pos);
        m_pathTail->Set_Layer(layer);
    }

    if (m_isOptimized) {
        if (m_pathTail != nullptr) {
            PathNode *node;
            for (node = m_pathHead; node && node->Get_Next_Optimized() != m_pathTail;
                 node = node->Get_Next_Optimized()) {
                ;
            }
            if (node) {
                if (node->Get_Next_Optimized() == m_pathTail) {
                    node->Set_Next_Optimized(m_pathTail);
                }
            }
        }
    }
}