/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Pathfinding
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
#include "pathfinder_zonemanager.h"
#include "snapshot.h"

class Path;
class PathfindZoneManager;
class Bridge;
class Locomotor;
class LocomotorSet;
class Object;
class PathfindCell;
class Weapon;
class PathfindLayer;

class PathfindServicesInterface
{
public:
    virtual Path *Find_Path(Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to) = 0;
    virtual Path *Find_Closest_Path(Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        Coord3D *raw_to,
        bool blocked,
        float path_cost_multiplier,
        bool move_allies) = 0;
    virtual Path *Find_Attack_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        const Object *victim,
        const Coord3D *victim_pos,
        const Weapon *weapon) = 0;
    virtual Path *Patch_Path(const Object *obj, const LocomotorSet &locomotor_set, Path *path, bool blocked) = 0;
    virtual Path *Find_Safe_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *pos1,
        const Coord3D *pos2,
        const Coord3D *pos3,
        float f) = 0;
};

class PathfindLayer
{
public:
    PathfindLayer();
    ~PathfindLayer();
    void Reset();

    bool Is_Unused();

    bool Init(Bridge *bridge, PathfindLayerEnum layer);

    bool Connects_Zones(PathfindZoneManager *manager, LocomotorSet *loco, int zone1, int zone2);

    bool Set_Destroyed(bool state);
    bool Is_Destroyed() { return m_destroyed; }
    
    void Apply_Zone();

    ObjectID Get_Bridge_ID();

    PathfindCell *Get_Cell(int x, int y);

    void Set_Zone(unsigned int zone) { m_zone = zone; }
    unsigned int Get_Zone() { return m_zone; }

    void Get_UnkCell1(ICoord2D &cell) { cell = m_unk1; }
    void Get_UnkCell2(ICoord2D &cell) { cell = m_unk1; }

private:
    PathfindCell *m_cells; // confirmed
    PathfindCell **m_layerCells; // confirmed
    int m_width; // confirmed
    int m_height; // confirmed
    int m_xOrigin; // confirmed
    int m_yOrigin; // confirmed
    ICoord2D m_unk1; // not 100% confirmed, BFME2 shows its two ICoord2D
    ICoord2D m_unk2; // not 100% confirmed
    PathfindLayerEnum m_layer; // confirmed
    int m_zone; // confirmed
    Bridge *m_bridge; // confirmed
    bool m_destroyed; // confirmed
};

class Pathfinder : public PathfindServicesInterface, public SnapShot
{
public:
#ifdef GAME_DLL
    Pathfinder *Hook_Ctor() { return new (this) Pathfinder(); }
    void Hook_Dtor() { Pathfinder::~Pathfinder(); }
#endif

    Pathfinder();
    ~Pathfinder();

    PathfindCell *Get_Cell(PathfindLayerEnum layer, int x, int y);

    virtual Path *Find_Path(
        Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to) override;
    virtual Path *Find_Closest_Path(Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        Coord3D *raw_to,
        bool blocked,
        float path_cost_multiplier,
        bool move_allies) override;
    virtual Path *Find_Attack_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        const Object *victim,
        const Coord3D *victim_pos,
        const Weapon *weapon) override;
    virtual Path *Patch_Path(const Object *obj, const LocomotorSet &locomotor_set, Path *path, bool blocked) override;
    virtual Path *Find_Safe_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *pos1,
        const Coord3D *pos2,
        const Coord3D *pos3,
        float f) override;
    virtual Path *Internal_Find_Path(
        Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to);

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Reset();

    bool Valid_Movement_Terrain(PathfindLayerEnum layer, const Locomotor *locomotor, const Coord3D *pos);
    bool Adjust_Target_Destination(const Object *source_obj,
        const Object *target_obj,
        const Coord3D *target_pos,
        const Weapon *weapon,
        Coord3D *destination_pos);

private:
    PathfindCell *m_mapPointer; // not 100% confirmed
    PathfindCell **m_map; // confirmed
    IRegion2D m_extent; // confirmed
    IRegion2D m_logicalExtent; // confirmed
    PathfindCell *m_openList; // confirmed
    PathfindCell *m_closedList; // confirmed
    bool m_isMapReady; // confirmed
    bool m_isTunneling; // confirmed
    int m_frameToShowObstacles; // confirmed
    Coord3D m_debugPathPos; // confirmed
    Path *m_debugPath; // confirmed
    ObjectID m_ignoreObstacleID; // confirmed
    PathfindZoneManager m_zoneManager; // confirmed
    PathfindLayer m_layers[LAYER_COUNT]; // confirmed
    ObjectID m_wallPieces[128]; // confirmed
    int m_numWallPieces; // confirmed
    float m_wallHeight; // confirmed
    int m_unk; // not 100% confirmed
    ObjectID m_queuedPathfindRequests[512]; //  confirmed
    int m_queuePRHead; // confirmed
    int m_queuePRTail; // confirmed
    int m_cumulativeCellsAllocated; // confirmed
};

inline PathfindCell *Pathfinder::Get_Cell(PathfindLayerEnum layer, int x, int y)
{
    if (x >= m_extent.lo.x && x <= m_extent.hi.x && y >= m_extent.lo.y && y <= m_extent.hi.y) {
        if (layer > LAYER_GROUND && layer <= LAYER_WALLS)
        {
            PathfindCell *cell = m_layers[layer].Get_Cell(x, y);
            if (cell != nullptr) {
                return cell;
            }
        }

        return &m_map[x][y];
    }

    return nullptr;
}
