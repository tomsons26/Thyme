/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Pathfinder Zone Manager
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

class PathfindCell;
class PathfindLayer;

class ZoneBlock
{
public:
    ZoneBlock();
    ~ZoneBlock();

    void Free_Zones();

    void Block_Calculate_Zones(PathfindCell **map, PathfindLayer *layer, const IRegion2D &bounds);
    unsigned short Get_Effective_Zone(int surfaces, bool bool1, unsigned short zone);

    void Allocate_Zones();

    void Set_Bridge(bool state) { m_bridge = state; }
    void Set_Passable(bool state) { m_passable = state; }

    bool Get_Bridge() { return m_bridge; }
    bool Get_Passable() { return m_passable; }

private:
    int m_x; // not 100% confirmed
    int m_y; // not 100% confirmed
    unsigned short m_firstBlockZone;
    unsigned short m_numBlockZones;
    unsigned short m_blockZoneSize; // confirmed
    unsigned short *m_groundCliffBlockZones; // not 100% confirmed
    unsigned short *m_groundWaterBlockZones; // not 100% confirmed
    unsigned short *m_groundRubbleBlockZones; // not 100% confirmed
    unsigned short *m_terrainBlockZones; // not 100% confirmed
    bool m_bridge; // not 100% confirmed
    bool m_passable; // not 100% confirmed
};

class PathfindZoneManager
{
public:
    PathfindZoneManager();
    ~PathfindZoneManager();

    void Clear_Passable_Flags();
    bool Clip_Is_Passable(int cellX, int cellY);

    void Free_Blocks();
    void Free_Zones();

    unsigned short Get_Block_Zone(int surfaces, bool use_lut, int cellx, int celly, PathfindCell **map);
    unsigned short Get_Effective_Terrain_Zone(unsigned short zone);
    unsigned short Get_Effective_Zone(int surfaces, bool bool1, unsigned short zone);

    bool Interacts_With_Bridge(int cellx, int celly);

    bool Is_Passable(int cellx, int celly);

    void Mark_Zones_Dirty(bool add);
    void Calculate_Zones(PathfindCell **map, PathfindLayer *layers, const IRegion2D &bounds);

    void Reset();

    void Set_All_Passable();

    void Set_Bridge(int cellx, int celly, bool state);

    void Set_Passable(int cellx, int celly, bool state);

    void Get_Zone_Block_Extent(ICoord2D *extent);

    bool Is_To_Update_Zones();

protected:
    void Allocate_Blocks(const IRegion2D &region);
    void Allocate_Zones();
    void Update_Zones_For_Modify(
        PathfindCell **map, PathfindLayer *layers, const IRegion2D &bounds1, const IRegion2D &bounds2);

private:
    ZoneBlock *m_zoneBlocks; // not 100% confirmed
    ZoneBlock **m_visibleZoneBlocks; // not 100% confirmed
    ICoord2D m_zoneBlockExtent; // confirmed
    unsigned short m_maxZone; // confirmed
    unsigned int m_frameToUpdateZones; // not 100% confirmed
    unsigned short m_zoneTableSize; // not 100% confirmed

    unsigned short *m_groundCliffZones; // confirmed
    unsigned short *m_groundWaterZones; // confirmed
    unsigned short *m_groundRubbleZones; // confirmed

    unsigned short *m_groundZonesUnknown1; // confirmed
    unsigned short *m_groundZonesUnknown2; // not 100% confirmed

    unsigned short *m_terrainZones; // confirmed
};
