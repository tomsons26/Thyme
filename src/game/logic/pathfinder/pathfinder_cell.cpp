/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Pathfinder Cell
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "pathfinder_cell.h"

#ifndef GAME_DLL
PathfindCellInfo *PathfindCellInfo::m_infoArray;
PathfindCellInfo *PathfindCellInfo::m_firstFree;
#endif

void PathfindCellInfo::Allocate_Cell_Infos()
{
    Release_Cell_Infos();
    m_infoArray = new PathfindCellInfo[MAX_CELL_INFOS];
    m_infoArray[MAX_CELL_INFOS - 1].m_pathParent = 0;
    m_infoArray[MAX_CELL_INFOS - 1].m_isFree = true;
    m_firstFree = m_infoArray;

    for (int i = 0; i < MAX_CELL_INFOS - 1; i++) {
        m_infoArray[i].m_pathParent = &m_infoArray[i + 1];
        m_infoArray[i].m_isFree = true;
    }
}

void PathfindCellInfo::Release_Cell_Infos()
{
    if (m_infoArray != nullptr) {
        int count = 0;

        while (m_firstFree != nullptr) {
            count++;
            captainslog_dbgassert(m_firstFree->m_isFree, "Should be freed.");
            m_firstFree = m_firstFree->m_pathParent;
        }

        captainslog_dbgassert(count == MAX_CELL_INFOS, "Error - Allocated cellinfos.");
        delete[] m_infoArray;
        m_infoArray = nullptr;
        m_firstFree = nullptr;
    }
}

void PathfindCell::Reset()
{
#if 0
    BYTE2(this->bits) &= 0xF0u;
    BYTE2(v1->bits) &= 0xFu;
    LOWORD(v1->bits) &= 0xC000u;
    LOWORD(v1->bits) &= 0xBFFFu;
    LOWORD(v1->bits) &= 0x7FFFu;
    if (this->m_info) {
        this->m_info->m_obstacleID = 0;
        PathfindCellInfo::releaseACellInfo(this->m_info);
        v1->m_info = 0;
    }
    HIBYTE(v1->bits) &= 0xF0u;
    HIBYTE(v1->bits) = HIBYTE(v1->bits) & 0xF | 0x10;
#endif
}

bool PathfindCell::Start_Pathfind(PathfindCell *goal_cell)
{
#if 0
    PathfindCell *v3; // [esp+0h] [ebp-4h]

    v3 = this;
    if (!this->m_info && !byte_E28160) {
        TheCurrentAllowCrashPtr = &byte_E28160;
        DebugCrash("Has to have info.");
        TheCurrentAllowCrashPtr = 0;
    }
    v3->m_info->m_next = 0;
    v3->m_info->m_prev = 0;
    v3->m_info->m_pathParent = 0;
    v3->m_info->m_costSoFar = 0;
    v3->m_info->m_totalCost = 0;
    if (goalcell) {
        v3->m_info->m_totalCost = PathfindCell::costToGoal(v3, goalcell);
    }
    v3->m_info->bitflags |= 0x10u;
    v3->m_info->bitflags &= 0xFFFFFFDF;
#endif
    return 1;
}

void PathfindCell::Set_Parent_Cell(PathfindCell *parent)
{
    if (!this->m_info && !byte_E28161) {
        TheCurrentAllowCrashPtr = &byte_E28161;
        DebugCrash("Has to have info.");
        TheCurrentAllowCrashPtr = 0;
    }

    this->m_info->m_pathParent = parent->m_info;
    int x = this->m_info->m_pos.x - parent->m_info->m_pos.x;
    int y = this->m_info->m_pos.y - parent->m_info->m_pos.y;
    if ((x < (signed int)-1u || x > 1 || y < (signed int)-1u || y > 1) && !byte_E28162) {
        TheCurrentAllowCrashPtr = &byte_E28162;
        DebugCrash("Invalid parent index.");
        TheCurrentAllowCrashPtr = 0;
    }
}

void PathfindCell::Set_Parent_Cell_Hierarchical(PathfindCell *parent)
{
    if (!this->m_info && !byte_E28163) {
        TheCurrentAllowCrashPtr = &byte_E28163;
        DebugCrash("Has to have info.");
        TheCurrentAllowCrashPtr = 0;
    }
    this->m_info->m_pathParent = parent->m_info;
}

void PathfindCell::Clear_Parent_Cell()
{
    if (!this->m_info && !byte_E28164) {
        TheCurrentAllowCrashPtr = &byte_E28164;
        DebugCrash("Has to have info.");
        TheCurrentAllowCrashPtr = 0;
    }
    this->m_info->m_pathParent = 0;
}

bool PathfindCell::Allocate_Info(const ICoord2D &pos)
{
    if (m_info != nullptr) {
        return true;
    }

    m_info = Get_A_Cell_Info(pos);
    return m_info != nullptr;
}

void PathfindCell::Set_Type(CellType type)
{
    if (this->m_info && this->m_info->m_obstacleID) {
        if (type != CELL_OBSTACLE && !byte_E28175) {
            TheCurrentAllowCrashPtr = &byte_E28175;
            DebugCrash("Wrong type.");
            TheCurrentAllowCrashPtr = 0;
        }
        BYTE2(this->bits) = BYTE2(this->bits) & 0xF0 | 4;
    } else {
        BYTE2(this->bits) = type & 0xF | BYTE2(this->bits) & 0xF0;
    }
}

bool PathfindCell::Remove_Obstacle(Object *obstacle)
{
    if ((BYTE2(this->bits) & 0xF) == CELL_RUBBLE) {
        BYTE2(this->bits) &= ~0xFu;
    }
    if (!this->m_info) {
        return 0;
    }
    v3 = this->m_info;
    if (v3->m_obstacleID != Object::getID(obstacle)) {
        return 0;
    }
    BYTE2(this->bits) &= ~0xFu;
    this->m_info->m_obstacleID = 0;
    PathfindCell::releaseInfo(this);
}

int PathfindCell::Cost_To_Goal(PathfindCell *goal)
{
    if (m_info == nullptr) {
        //if (!byte_E28184) {
        //    TheCurrentAllowCrashPtr = &byte_E28184;
        //    DebugCrash("Has to have info.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    int cost;
    int x = m_info->m_pos.x - goal->Get_X_Index();
    int y = m_info->m_pos.y - goal->Get_Y_Index();

    if (x < 0) {
        x = -x;
    }
    if (y < 0) {
        y = -y;
    }

    if (x > y) {
        cost = 10 * y / 2 + 10 * x;
    } else {
        cost = 10 * x / 2 + 10 * y;
    }

    return cost;
}

int PathfindCell::Cost_To_Hierarchical_Goal(PathfindCell *goal)
{
    if (m_info = nullptr) {
        // if (!byte_E28185) {
        //    TheCurrentAllowCrashPtr = &byte_E28185;
        //    DebugCrash("Has to have info.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return 100000;
    }

    int x = m_info->m_pos.x - goal->Get_X_Index();
    int y = m_info->m_pos.y - goal->Get_Y_Index();

    float v1 = sqrt(y * y + x * x) * 10.0 + 0.5;
    float v8 = fast_float_floor(v1);
    int cost = sagemath_lrintf(v8);
    return cost;
}

int PathfindCell::Cost_So_Far(PathfindCell *parent)
{
    if (m_info == nullptr) {
        // if (!byte_E28186) {
        //    TheCurrentAllowCrashPtr = &byte_E28186;
        //    DebugCrash("Has to have info.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }
    if (parent == nullptr) {
        return 0;
    }

    int x1 = parent->Get_X_Index() - m_info->m_pos.x;
    int y1 = parent->Get_Y_Index() - m_info->m_pos.y;

    int cost1;
    if (!x1 || !y1) {
        cost1 = parent->Get_Cost_So_Far() + 10;
    } else {
        cost1 = parent->Get_Cost_So_Far() + 14;
    }
    if (Get_Unk2()) {
        cost1 += 14;
    }

    int cost2 = 0;
    PathfindCell *cell = parent->Get_Parent_Cell();

    if (cell != nullptr) {
        int x2 = cell->Get_X_Index() - parent->Get_X_Index();
        int y2 = cell->Get_Y_Index() - parent->Get_Y_Index();

        if (x2 != x1 || y2 != y1) {
            int dist = y1 * y2 + x1 * x2;

            if (dist > 0) {
                cost2 = 4;
            } else {
                if (dist == 0) {
                    cost2 = 8;
                } else {
                    cost2 = 16;
                }
            }
        }
    }

    return cost2 + cost1;
}
