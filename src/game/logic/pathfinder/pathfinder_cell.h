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
#pragma once
#include "always.h"
#include "coord.h"
#include "gametype.h"

class Object;
class PathfindCell;

class PathfindCellInfo
{
    friend PathfindCell;

public:
    static void Allocate_Cell_Infos();
    static void Release_Cell_Infos();

private:
    enum
    {
        MAX_CELL_INFOS = 30000,
    };

    PathfindCellInfo *m_next; // confirmed
    PathfindCellInfo *m_prev; // confirmed
    PathfindCellInfo *m_pathParent; // confirmed
    PathfindCell *m_cell; // confirmed
    unsigned short m_totalCost; // confirmed
    unsigned short m_costSoFar; // confirmed
    ICoord2D m_pos; // confirmed
    ObjectID m_goalUnitID; // confirmed
    ObjectID m_posUnitID; // confirmed
    ObjectID m_goalAircraftID; // confirmed
    ObjectID m_obstacleID; // confirmed
    bool m_isFree : 1; // confirmed
    bool m_blockedByAlly : 1; // confirmed
    bool m_unk2 : 1; // not 100% confirmed
    bool m_unk3 : 1; // not 100% confirmed
    bool m_open : 1; // confirmed
    bool m_closed : 1; // confirmed

#ifdef GAME_DLL
    static PathfindCellInfo *&m_infoArray;
    static PathfindCellInfo *&m_firstFree;
#else
    static PathfindCellInfo *m_infoArray;
    static PathfindCellInfo *m_firstFree;
#endif
};

class PathfindCell
{
public:
    enum CellType
    {
        CELL_CLEAR, // confirmed
        CELL_WATER, // confirmed
        CELL_CLIFF, // confirmed
        CELL_RUBBLE, // confirmed
        CELL_OBSTACLE, // confirmed
        CELL_TYPE_5,
        CELL_TYPE_6,
    };

    enum CellFlags
    {
        NO_UNITS = 0,
        UNIT_GOAL = 1,
        UNIT_PRESENT_MOVING = 2,
        UNIT_PRESENT_FIXED = 3,
        UNIT_GOAL_OTHER_MOVING = 5,
    };

    PathfindCell();
    ~PathfindCell();

    void Reset();

    bool Start_Pathfind(PathfindCell *goal_cell);

    void Set_Parent_Cell(PathfindCell *parent);
    void Set_Parent_Cell_Hierarchical(PathfindCell *parent);
    void Clear_Parent_Cell();

    bool Allocate_Info(const ICoord2D &pos);
    bool Release_Info();

    void Set_Goal_Unit(ObjectID id, const ICoord2D &pos);
    void Set_Goal_Aircraft(ObjectID id, const ICoord2D &pos);
    void Set_Pos_Unit(ObjectID id, const ICoord2D &pos);
    void Set_Type_As_Obstacle(Object *obstacle, bool bool1, const ICoord2D &pos);

    void Set_Type(CellType type);
    CellType Get_Type() { return (CellType)m_type; }

    bool Remove_Obstacle(Object *obstacle);

    PathfindCell *Put_On_Sorted_Open_List(PathfindCell *list);
    PathfindCell *Remove_From_Open_List(PathfindCell *list);

    static int Release_Open_List(PathfindCell *list);
    static int Release_Closed_List(PathfindCell *list);

    PathfindCell *Put_On_Closed_List(PathfindCell *list);
    PathfindCell *Reemove_From_Closed_List(PathfindCell *list);

    int Cost_To_Goal(PathfindCell *goal);
    int Cost_To_Hierarchical_Goal(PathfindCell *goal);
    int Cost_So_Far(PathfindCell *parent);

    unsigned short Get_Cost_So_Far() { m_info->m_costSoFar; }

    int Get_X_Index() { return m_info->m_pos.x; }
    int Get_Y_Index() { return m_info->m_pos.x; }

    PathfindCell *Get_Parent_Cell();

    unsigned short Get_Temp_Zone() { return m_zone; }
    void Set_Temp_Zone(unsigned short zone) { m_zone = zone; }

    PathfindLayerEnum Get_Connect_Layer() { return (PathfindLayerEnum)m_connectLayer; }
    void Set_Connect_Layer(PathfindLayerEnum layer) { m_connectLayer = layer; }

    PathfindLayerEnum Get_Unknown_Layer() { return (PathfindLayerEnum)m_layer2; }
    void Set_Unknown_Layer(PathfindLayerEnum layer) { m_layer2 = layer; }

    bool Get_Unk2() { return m_unk2; }
    void Set_Unk2(bool state) { m_unk2 = state; }

private:
    PathfindCellInfo *m_info; // confirmed
    unsigned short m_zone : 14; // confirmed
    unsigned char m_aircraftGoal : 1; // confirmed
    unsigned char m_unk2 : 1; // not 100% confirmed
    unsigned char m_type : 4; // confirmed
    unsigned char m_flags : 4; // not 100% confirmed
    unsigned char m_connectLayer : 4; // not 100% confirmed
    unsigned char m_layer2 : 4; // not 100% confirmed
};

inline PathfindCell *PathfindCell::Get_Parent_Cell()
{
    if (m_info != nullptr) {
        if (m_info->m_pathParent != nullptr) {
            return m_info->m_pathParent->m_cell;
        }
    }

    return nullptr;
}
