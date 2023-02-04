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
#include "pathfinder_zonemanager.h"

#include "aipathfind.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "locomotor.h"
#include "pathfinder_cell.h"
#include "terrainlogic.h"
#include "w3ddebugicons.h"

bool PathfindCellCompareType1(PathfindCell *cell1, PathfindCell *cell2)
{
    PathfindCell::CellType type1 = cell1->Get_Type();
    PathfindCell::CellType type2 = cell2->Get_Type();

    if (type1 == PathfindCell::CELL_CLEAR && type2 & PathfindCell::CELL_WATER) {
        return true;
    }

    if (type2 == PathfindCell::CELL_CLEAR && type1 & PathfindCell::CELL_WATER) {
        return true;
    }

    return false;
}

bool PathfindCellCompareType3(PathfindCell *cell1, PathfindCell *cell2)
{
    PathfindCell::CellType type1 = cell1->Get_Type();
    PathfindCell::CellType type2 = cell2->Get_Type();

    if (type1 == PathfindCell::CELL_CLEAR && type2 == PathfindCell::CELL_RUBBLE) {
        return true;
    }

    if (type2 == PathfindCell::CELL_CLEAR && type1 == PathfindCell::CELL_RUBBLE) {
        return true;
    }

    return false;
}

bool PathfindCellCompareType4_Flag(PathfindCell *cell1, PathfindCell *cell2)
{
    PathfindCell::CellType type1 = cell1->Get_Type();
    PathfindCell::CellType type2 = cell2->Get_Type();

    if (type1 == PathfindCell::CELL_OBSTACLE && cell1->Get_Unk2() && type2 == PathfindCell::CELL_CLEAR) {
        return true;
    }

    if (type2 == PathfindCell::CELL_OBSTACLE && cell2->Get_Unk2() && type1 == PathfindCell::CELL_CLEAR) {
        return true;
    }

    return false;
}

bool PathfindCellCompareType2(PathfindCell *cell1, PathfindCell *cell2)
{
    PathfindCell::CellType type1 = cell1->Get_Type();
    PathfindCell::CellType type2 = cell2->Get_Type();

    if (type1 == PathfindCell::CELL_CLIFF && type2 == PathfindCell::CELL_CLEAR) {
        return true;
    }

    if (type1 == PathfindCell::CELL_CLEAR && type2 == PathfindCell::CELL_CLIFF) {
        return true;
    }

    return false;
}

bool PathfindCellCompareType4(PathfindCell *cell1, PathfindCell *cell2)
{
    PathfindCell::CellType type1 = cell1->Get_Type();
    PathfindCell::CellType type2 = cell2->Get_Type();

    if (type1 == PathfindCell::CELL_OBSTACLE) {
        type1 = PathfindCell::CELL_CLEAR;
    }

    if (type2 == PathfindCell::CELL_OBSTACLE) {
        type2 = PathfindCell::CELL_CLEAR;
    }

    return type1 == type2;
}

bool PathfindCellCompareTypes(PathfindCell *cell1, PathfindCell *cell2)
{
    PathfindCell::CellType type1 = cell1->Get_Type();
    PathfindCell::CellType type2 = cell2->Get_Type();

    return type1 == type2;
}

ZoneBlock::ZoneBlock() :
    m_x(0),
    m_y(0),
    m_firstBlockZone(0),
    m_numBlockZones(0),
    m_blockZoneSize(0),
    m_groundCliffBlockZones(nullptr),
    m_groundWaterBlockZones(nullptr),
    m_groundRubbleBlockZones(nullptr),
    m_terrainBlockZones(nullptr),
    m_bridge(false),
    m_passable(true)
{
}

ZoneBlock::~ZoneBlock()
{
    Free_Zones();
}

void ZoneBlock::Free_Zones()
{
    if (m_groundCliffBlockZones != nullptr) {
        delete[] m_groundCliffBlockZones;
        m_groundCliffBlockZones = nullptr;
    }

    if (m_groundWaterBlockZones != nullptr) {
        delete[] m_groundWaterBlockZones;
        m_groundWaterBlockZones = nullptr;
    }

    if (m_groundRubbleBlockZones != nullptr) {
        delete[] m_groundRubbleBlockZones;
        m_groundRubbleBlockZones = nullptr;
    }

    if (m_terrainBlockZones != nullptr) {
        delete[] m_terrainBlockZones;
        m_terrainBlockZones = nullptr;
    }

    // BUGFIX
    m_blockZoneSize = 0;
}

void Resolve_Block_Zones(int srcZone, int targetZone, unsigned short *zoneEquivalency, int sizeOfZE)
{
    int trgt = targetZone;
    int src = srcZone;
    if (!srcZone || !targetZone) {
        // if (!byte_E28187) {
        //    TheCurrentAllowCrashPtr = &byte_E28187;
        //    DebugCrash("Bad resolve zones\t.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    if (trgt < src) {
        for (int i = 0; i < sizeOfZE; ++i) {
            if (zoneEquivalency[i] == src) {
                zoneEquivalency[i] = trgt;
            }
        }
    } else {
        for (int i = 0; i < sizeOfZE; ++i) {
            if (zoneEquivalency[i] == trgt) {
                zoneEquivalency[i] = src;
            }
        }
    }
}

void Apply_Block_Zone(PathfindCell *targetCell,
    PathfindCell *sourceCell,
    unsigned short *zoneEquivalency,
    int firstZone,
    unsigned int sizeOfZE)
{
    if (sourceCell->Get_Temp_Zone() < firstZone || sourceCell->Get_Temp_Zone() >= sizeOfZE + firstZone) {
        // if (!byte_E281F8) {
        //    TheCurrentAllowCrashPtr = &byte_E281F8;
        //    DebugCrash("Memory overrun - FATAL ERROR.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    int srcZone = zoneEquivalency[sourceCell->Get_Temp_Zone() - firstZone];

    if (targetCell->Get_Temp_Zone() < firstZone || sourceCell->Get_Temp_Zone() >= sizeOfZE + firstZone) {
        // if (!byte_E281F4) {
        //    TheCurrentAllowCrashPtr = &byte_E281F4;
        //    DebugCrash("Memory overrun - FATAL ERROR.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    int targetZone = zoneEquivalency[targetCell->Get_Temp_Zone() - firstZone];

    if (targetZone != srcZone) {
        Resolve_Block_Zones(srcZone, targetZone, zoneEquivalency, sizeOfZE);
    }
}

void ZoneBlock::Block_Calculate_Zones(PathfindCell **map, PathfindLayer *const layer, const IRegion2D &bounds)
{
    m_x = bounds.lo.x;
    m_y = bounds.lo.y;

    int zone_1 = map[bounds.lo.x][bounds.lo.y].Get_Temp_Zone();
    int zone_2 = zone_1;

    for (int x = bounds.lo.y; x <= bounds.hi.y; ++x) {
        for (int y = bounds.lo.x; y <= bounds.hi.x; ++y) {

            unsigned short temp_zone = map[x][y].Get_Temp_Zone();
            if (zone_1 > temp_zone) {
                zone_1 = temp_zone;
            }
            if (zone_2 < temp_zone) {
                zone_2 = temp_zone;
            }
        }
    }
    m_firstBlockZone = zone_1;
    m_numBlockZones = zone_2 + 1 - zone_1;

    Allocate_Zones();

    if (m_numBlockZones != 1) {
        for (unsigned short i = 0; i < m_blockZoneSize; ++i) {
            m_groundCliffBlockZones[i] = m_firstBlockZone + i;
            m_groundWaterBlockZones[i] = m_firstBlockZone + i;
            m_groundRubbleBlockZones[i] = m_firstBlockZone + i;
            m_terrainBlockZones[i] = m_firstBlockZone + i;
        }

        for (int y = bounds.lo.y; y <= bounds.hi.y; ++y) {
            for (int x = bounds.lo.x; x <= bounds.hi.x; ++x) {
                if (x > bounds.lo.x) {

                    if (map[x][y].Get_Temp_Zone() != map[x - 1][y].Get_Temp_Zone()) {

                        if (PathfindCellCompareType1(&map[x][y], &map[x - 1][y])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x - 1][y], m_groundWaterBlockZones, m_firstBlockZone, m_numBlockZones);
                        }

                        if (PathfindCellCompareType3(&map[x][y], &map[x - 1][y])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x - 1][y], m_groundRubbleBlockZones, m_firstBlockZone, m_numBlockZones);
                        }

                        if (PathfindCellCompareType2(&map[x][y], &map[x - 1][y])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x - 1][y], m_groundCliffBlockZones, m_firstBlockZone, m_numBlockZones);
                        }

                        if (PathfindCellCompareType4_Flag(&map[x][y], &map[x - 1][y])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x - 1][y], m_terrainBlockZones, m_firstBlockZone, m_numBlockZones);
                        }
                    }
                }
                if (y > bounds.lo.y) {
                    if (map[x][y].Get_Temp_Zone() != map[x][y - 1].Get_Temp_Zone()) {

                        if (PathfindCellCompareType1(&map[x][y], &map[x][y - 1])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x][y - 1], m_groundWaterBlockZones, m_firstBlockZone, m_numBlockZones);
                        }

                        if (PathfindCellCompareType3(&map[x][y], &map[x][y - 1])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x][y - 1], m_groundRubbleBlockZones, m_firstBlockZone, m_numBlockZones);
                        }

                        if (PathfindCellCompareType2(&map[x][y], &map[x][y - 1])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x][y - 1], m_groundCliffBlockZones, m_firstBlockZone, m_numBlockZones);
                        }

                        if (PathfindCellCompareType4_Flag(&map[x][y], &map[x][y - 1])) {
                            Apply_Block_Zone(
                                &map[x][y], &map[x][y - 1], m_terrainBlockZones, m_firstBlockZone, m_numBlockZones);
                        }
                    }
                }

                if (map[x][y].Get_Temp_Zone() == 0) {
                    // if (!byte_E2818C) {
                    //    TheCurrentAllowCrashPtr = &byte_E2818C;
                    //    DebugCrash("Cleared the zone.");
                    //    TheCurrentAllowCrashPtr = 0;
                    //}
                }
            }
        }
    }
}

unsigned short ZoneBlock::Get_Effective_Zone(int surfaces, bool bool1, unsigned short zone)
{
    if (zone == 0) {
        return 0;
    }

    if (surfaces & LOCOMOTOR_SURFACE_AIR) {
        return 1;
    }

    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_WATER && surfaces & LOCOMOTOR_SURFACE_CLIFF) {
        return 1;
    }

    if (m_numBlockZones < 2) {
        return m_firstBlockZone;
    }

    // if ((zone < m_firstBlockZone || zone >= m_numBlockZones + m_firstBlockZone) && !byte_E2818D) {
    //    TheCurrentAllowCrashPtr = &byte_E2818D;
    //    DebugCrash("Invalid range.");
    //    TheCurrentAllowCrashPtr = 0;
    //}

    if (zone < m_firstBlockZone || zone >= m_numBlockZones + m_firstBlockZone) {
        return m_firstBlockZone;
    }

    zone -= m_firstBlockZone;

    if (bool1) {
        zone = m_terrainBlockZones[zone];
        // if ((zone < m_firstBlockZone || zone >= m_numBlockZones + m_firstBlockZone) && !byte_E2818E) {
        //    TheCurrentAllowCrashPtr = &byte_E2818E;
        //    DebugCrash("Invalid range.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        zone -= m_firstBlockZone;
    }

    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_CLIFF) {
        zone = m_groundCliffBlockZones[zone];
        // if ((zone < m_firstBlockZone || zone >= m_numBlockZones + m_firstBlockZone) && !byte_E2818F) {
        //    TheCurrentAllowCrashPtr = &byte_E2818F;
        //    DebugCrash("Invalid range.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return zone;
    }

    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_WATER) {
        zone = m_groundWaterBlockZones[zone];
        // if ((zone < m_firstBlockZone || zone >= m_numBlockZones + m_firstBlockZone) && !byte_E28190) {
        //    TheCurrentAllowCrashPtr = &byte_E28190;
        //    DebugCrash("Invalid range.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return zone;
    }

    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_RUBBLE) {
        return m_groundRubbleBlockZones[zone];
    }

    if (surfaces & LOCOMOTOR_SURFACE_CLIFF) {
        if (surfaces & LOCOMOTOR_SURFACE_WATER) {
            // if (!byte_E28191) {
            //    TheCurrentAllowCrashPtr = &byte_E28191;
            //    DebugCrash("Cliff water only locomotor sets not supported yet.");
            //    TheCurrentAllowCrashPtr = 0;
            //}
        }
    }

    return m_firstBlockZone + zone;
}

void ZoneBlock::Allocate_Zones()
{
    if (m_blockZoneSize <= m_numBlockZones || !m_groundCliffBlockZones) {
        Free_Zones();

        if (m_numBlockZones != 1) {
            if (m_blockZoneSize == 0) {
                m_blockZoneSize = 4;
            }
            while (m_blockZoneSize <= m_numBlockZones) {
                m_blockZoneSize *= 2;
            }

            m_groundCliffBlockZones = new unsigned short[m_blockZoneSize];
            m_groundWaterBlockZones = new unsigned short[m_blockZoneSize];
            m_groundRubbleBlockZones = new unsigned short[m_blockZoneSize];
            m_terrainBlockZones = new unsigned short[m_blockZoneSize];
        }
    }
}

PathfindZoneManager::PathfindZoneManager() :
    m_zoneBlocks(nullptr),
    m_visibleZoneBlocks(nullptr),
    m_maxZone(0),
    m_frameToUpdateZones(0),
    m_zoneTableSize(0),
    m_groundCliffZones(nullptr),
    m_groundWaterZones(nullptr),
    m_groundRubbleZones(nullptr),
    m_groundZonesUnknown1(nullptr),
    m_groundZonesUnknown2(nullptr),
    m_terrainZones(nullptr)
{
    m_zoneBlockExtent.x = 0;
    m_zoneBlockExtent.y = 0;
}

void PathfindZoneManager::Allocate_Blocks(const IRegion2D &region)
{
    Free_Blocks();

    m_zoneBlockExtent.x = (region.hi.x - region.lo.x + 10) / 10;
    m_zoneBlockExtent.y = (region.hi.y - region.lo.y + 10) / 10;

    int size = m_zoneBlockExtent.y * m_zoneBlockExtent.x;
    m_zoneBlocks = new ZoneBlock[size];
    m_visibleZoneBlocks = new ZoneBlock *[m_zoneBlockExtent.x];

    for (int x = 0; x < m_zoneBlockExtent.x; ++x) {
        m_visibleZoneBlocks[x] = &m_zoneBlocks[x * m_zoneBlockExtent.y];
    }
}

void PathfindZoneManager::Allocate_Zones()
{
    if (m_zoneTableSize <= m_maxZone || !m_groundCliffZones) {
        Free_Zones();

        if (m_zoneTableSize == 0) {
            m_zoneTableSize = 256;
        }

        while (m_zoneTableSize <= m_maxZone) {
            m_zoneTableSize *= 2;
        }

        captainslog_debug("Allocating zone tables of size %d", m_zoneTableSize);

        m_groundCliffZones = new unsigned short[m_zoneTableSize];
        m_groundWaterZones = new unsigned short[m_zoneTableSize];
        m_groundRubbleZones = new unsigned short[m_zoneTableSize];
        m_groundZonesUnknown1 = new unsigned short[m_zoneTableSize];
        m_groundZonesUnknown2 = new unsigned short[m_zoneTableSize];
        m_terrainZones = new unsigned short[m_zoneTableSize];
    }
}

void PathfindZoneManager::Update_Zones_For_Modify(
    PathfindCell **map, PathfindLayer *layers, const IRegion2D &bounds1, const IRegion2D &bounds2)
{
    // LARGE_INTEGER Frequency;
    // LARGE_INTEGER PerformanceCount;
    // LARGE_INTEGER v32;

    // double v36 = 0.0;

    // QueryPerformanceFrequency(&Frequency);
    // QueryPerformanceCounter(&PerformanceCount);

    IRegion2D reg;
    reg.lo.x = bounds1.lo.x;
    reg.lo.y = bounds1.lo.y;
    reg.hi.x = bounds1.hi.x;
    reg.hi.y = bounds1.hi.y;
    ++reg.hi.x;
    ++reg.hi.y;
    if (reg.hi.x > bounds2.hi.x) {
        reg.hi.x = bounds2.hi.x;
    }
    if (reg.hi.y > bounds2.hi.y) {
        reg.hi.y = bounds2.hi.y;
    }

    unsigned short temp;

    for (int x = 0; x < m_zoneBlockExtent.x; ++x) {
        for (int y = 0; y < m_zoneBlockExtent.y; ++y) {

            IRegion2D region;
            region.lo.x = 10 * x + bounds2.lo.x;
            region.lo.y = 10 * y + bounds2.lo.y;
            region.hi.x = region.lo.x + 9;
            region.hi.y = region.lo.y + 9;

            if (region.lo.x + 9 > reg.hi.x) {
                region.hi.x = reg.hi.x;
            }
            if (region.hi.y > reg.hi.y) {
                region.hi.y = reg.hi.y;
            }
            if (region.lo.x < reg.lo.x) {
                region.lo.x = reg.lo.x;
            }
            if (region.lo.y < reg.lo.y) {
                region.lo.y = reg.lo.y;
            }

            if (region.lo.x <= region.hi.x && region.lo.y <= region.hi.y) {

                m_visibleZoneBlocks[x][y].Set_Bridge(false);

                for (int ry = region.lo.y; ry <= region.hi.y; ++ry) {
                    for (int rx = region.lo.x; rx <= region.hi.x; ++rx) {

                        PathfindCell *cell = &map[rx][ry];

                        if (cell->Get_Temp_Zone() == 0) {
                            if (rx <= region.lo.x || map[rx][ry].Get_Type() != map[rx - 1][ry].Get_Type()
                                || (temp = map[rx - 1][ry].Get_Temp_Zone(),
                                    cell->Set_Temp_Zone(temp),
                                    cell->Get_Temp_Zone() == 0)) {

                                if (ry > region.lo.y) {
                                    if (cell->Get_Type() != map[rx][ry - 1].Get_Type()
                                        || (temp = map[rx][ry - 1].Get_Temp_Zone(),
                                            cell->Set_Temp_Zone(temp),
                                            cell->Get_Temp_Zone() == 0)) {

                                        if (rx < region.hi.x && PathfindCellCompareTypes(cell, &map[rx + 1][ry - 1])
                                            && PathfindCellCompareTypes(cell, &map[rx + 1][ry])) {
                                            temp = map[rx + 1][ry - 1].Get_Temp_Zone();
                                            cell->Set_Temp_Zone(temp);
                                            cell->Get_Temp_Zone(); // TODO, BUG?
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                for (int ry = region.hi.y; ry >= region.lo.y; --ry) {
                    for (int rx = region.hi.x; rx >= region.lo.x; --rx) {
                        PathfindCell *cell = &map[rx][ry];

                        if (cell->Get_Temp_Zone() == 0) {
                            // TODO cleanup
                            if (rx >= region.hi.x || map[rx][ry].Get_Type() != map[rx + 1][ry].Get_Type()
                                || (temp = map[rx + 1][ry].Get_Temp_Zone(),
                                    cell->Set_Temp_Zone(temp),
                                    cell->Get_Temp_Zone() == 0)) {

                                if (ry < region.hi.y) {
                                    // TODO cleanup
                                    if (cell->Get_Type() != map[rx][ry + 1].Get_Temp_Zone()
                                        || (temp = map[rx][ry + 1].Get_Temp_Zone(),
                                            cell->Set_Temp_Zone(temp),
                                            cell->Get_Temp_Zone() == 0)) {

                                        if (rx < region.hi.x && PathfindCellCompareTypes(cell, &map[rx + 1][ry + 1])
                                            && PathfindCellCompareTypes(cell, &map[rx + 1][ry])) {
                                            temp = map[rx + 1][ry + 1].Get_Temp_Zone();
                                            cell->Set_Temp_Zone(temp);
                                            cell->Get_Temp_Zone(); // TODO, BUG?
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // QueryPerformanceCounter(&v32)
    // v36 = (double)(v32.QuadPart - PerformanceCount.QuadPart) / (double)Frequency.QuadPart;

    if (g_theWriteableGlobalData->m_debugAI == AI_DEBUG_LEVEL_5) {
        RGBColor color;
        Coord3D pos;
        float col[3];

        memset(col, 0, 4u);
        color.red = col[0];
        color.green = col[1];
        color.blue = col[2];

        Add_Icon(nullptr, 0.0, 0, color);
        for (int y = 0; y < bounds2.hi.y; ++y) {
            for (int x = 0; x < bounds2.hi.x; ++x) {
                int zone = m_terrainZones[map[x][y].Get_Temp_Zone()];
                col[0] = (double)(zone / 3 / 3 % 3) * 0.5;
                col[1] = (double)(zone / 3 % 3) * 0.5;
                col[2] = (double)(zone % 3) * 0.5;

                pos.x = ((double)x + 0.5) * 10.0;
                pos.y = ((double)y + 0.5) * 10.0;

                PathfindLayerEnum layer = map[x][y].Get_Unknown_Layer();
                pos.z = g_theTerrainLogic->Get_Layer_Height(pos.x, pos.y, layer, nullptr, true) + 0.5;
                color.red = col[0];
                color.green = col[1];
                color.blue = col[2];
                Add_Icon(&pos, 8.0, 200, color);
            }
        }
    }
}

void PathfindZoneManager::Clear_Passable_Flags()
{
    for (int x = 0; x < m_zoneBlockExtent.x; ++x) {
        for (int y = 0; y < m_zoneBlockExtent.y; ++y) {
            m_visibleZoneBlocks[x][y].Set_Passable(false);
        }
    }
}

bool PathfindZoneManager::Clip_Is_Passable(int cellX, int cellY)
{
    int x = cellX / 10;
    int y = cellY / 10;

    if ((x >= 0 && x < m_zoneBlockExtent.x) && (y >= 0 && y < m_zoneBlockExtent.y)) {
        return m_visibleZoneBlocks[x][y].Get_Passable();
    }

    return false;
}

void PathfindZoneManager::Free_Blocks()
{
    if (m_zoneBlocks != nullptr) {
        delete[] m_zoneBlocks;
        m_zoneBlocks = nullptr;
    }
    if (m_visibleZoneBlocks) {
        delete[] m_visibleZoneBlocks;
        m_visibleZoneBlocks = nullptr;
    }

    m_zoneBlockExtent.x = 0;
    m_zoneBlockExtent.y = 0;
}

void PathfindZoneManager::Free_Zones()
{
    if (m_groundCliffZones != nullptr) {
        delete[] m_groundCliffZones;
        m_groundCliffZones = nullptr;
    }

    if (m_groundWaterZones != nullptr) {
        delete[] m_groundWaterZones;
        m_groundWaterZones = nullptr;
    }

    if (m_groundRubbleZones != nullptr) {
        delete[] m_groundRubbleZones;
        m_groundRubbleZones = nullptr;
    }

    if (m_groundZonesUnknown1 != nullptr) {
        delete[] m_groundZonesUnknown1;
        m_groundZonesUnknown1 = nullptr;
    }

    if (m_groundZonesUnknown2 != nullptr) {
        delete[] m_groundZonesUnknown2;
        m_groundZonesUnknown2 = nullptr;
    }

    if (m_terrainZones != nullptr) {
        delete[] m_terrainZones;
        m_terrainZones = nullptr;
    }

    m_zoneTableSize = 0;
}

unsigned short PathfindZoneManager::Get_Block_Zone(int surfaces, bool use_lut, int cellx, int celly, PathfindCell **map)
{
    int x = cellx / 10;
    int y = celly / 10;

    if (x < 0 || x >= m_zoneBlockExtent.x) {
        // if (!byte_E281B2) {
        //    TheCurrentAllowCrashPtr = &byte_E281B2;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return 0;
    }

    if (y < 0 || y >= m_zoneBlockExtent.y) {
        // if (!byte_E281B3) {
        //    TheCurrentAllowCrashPtr = &byte_E281B3;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return 0;
    }

    unsigned short zone = m_visibleZoneBlocks[x][y].Get_Effective_Zone(surfaces, use_lut, map[cellx][celly].Get_Temp_Zone());

    if (zone >= m_maxZone) {
        // if (!byte_E281B4) {
        //    TheCurrentAllowCrashPtr = &byte_E281B4;
        //    DebugCrash("Invalid zone.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return 0;
    }

    return zone;
}

unsigned short PathfindZoneManager::Get_Effective_Terrain_Zone(unsigned short zone)
{
    return m_terrainZones[m_groundZonesUnknown1[zone]];
}

unsigned short PathfindZoneManager::Get_Effective_Zone(int surfaces, bool bool1, unsigned short zone)
{
    if (zone > m_maxZone) {
        // if (!byte_E281B5) {
        //    TheCurrentAllowCrashPtr = &byte_E281B5;
        //    DebugCrash("Invalid zone");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return 0;
    }

    if (zone > m_maxZone) {
        // if (!byte_E281B6) {
        //    TheCurrentAllowCrashPtr = &byte_E281B6;
        //    DebugCrash("Invalid zone");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return 0;
    }

    if (surfaces & LOCOMOTOR_SURFACE_AIR) {
        return 1;
    }
    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_WATER && surfaces & LOCOMOTOR_SURFACE_CLIFF) {
        return 1;
    }

    if (bool1) {
        zone = m_groundZonesUnknown2[zone];
    }

    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_CLIFF) {
        return m_groundCliffZones[zone];
    }
    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_WATER) {
        return m_groundWaterZones[zone];
    }
    if (surfaces & LOCOMOTOR_SURFACE_GROUND && surfaces & LOCOMOTOR_SURFACE_RUBBLE) {
        return m_groundRubbleZones[zone];
    }

    if (surfaces & LOCOMOTOR_SURFACE_CLIFF && surfaces & LOCOMOTOR_SURFACE_WATER) {
        // if (!byte_E281B7) {
        //    TheCurrentAllowCrashPtr = &byte_E281B7;
        //    DebugCrash("Cliff water only locomotor sets not supported yet.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    return m_terrainZones[zone];
}

bool PathfindZoneManager::Interacts_With_Bridge(int cellx, int celly)
{
    int x = cellx / 10;
    int y = celly / 10;

    if (x < 0 || x >= m_zoneBlockExtent.x) {
        // if (!byte_E281AE) {
        //    TheCurrentAllowCrashPtr = &byte_E281AE;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return false;
    }
    if (y < 0 || y >= m_zoneBlockExtent.y) {
        // if (!byte_E281AF) {
        //    TheCurrentAllowCrashPtr = &byte_E281AF;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return false;
    }

    return m_visibleZoneBlocks[x][y].Get_Bridge();
}

bool PathfindZoneManager::Is_Passable(int cellx, int celly)
{
    int x = cellx / 10;
    int y = celly / 10;

    if (x < 0 || x >= m_zoneBlockExtent.x) {
        // if (!byte_E281AE) {
        //    TheCurrentAllowCrashPtr = &byte_E281AE;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return false;
    }

    if (y < 0 || y >= m_zoneBlockExtent.y) {
        // if (!byte_E281AF) {
        //    TheCurrentAllowCrashPtr = &byte_E281AF;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
        return false;
    }

    return m_visibleZoneBlocks[x][y].Get_Passable();
}

unsigned int ZONE_UPDATE_FREQUENCY = 300;

void PathfindZoneManager::Mark_Zones_Dirty(bool add)
{
    unsigned int frame;

    if (g_theGameLogic->Get_Frame() < 2) {
        m_frameToUpdateZones = 2;
    } else {
        if (m_frameToUpdateZones < ZONE_UPDATE_FREQUENCY + g_theGameLogic->Get_Frame()) {
            frame = m_frameToUpdateZones;
        } else {
            frame = ZONE_UPDATE_FREQUENCY + g_theGameLogic->Get_Frame();
        }
        m_frameToUpdateZones = frame;
    }
}

void Resolve_Zones(int srcZone, int targetZone, unsigned short *zoneEquivalency, int sizeOfZE)
{
    unsigned short new_zone;

    if ((!srcZone || !targetZone)) {
        // if (!byte_E28188) {
        //    TheCurrentAllowCrashPtr = &byte_E28188;
        //    DebugCrash("Bad resolve zones\t.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    if ((srcZone >= sizeOfZE || targetZone >= sizeOfZE)) {
        // if (!byte_E28189) {
        //    TheCurrentAllowCrashPtr = &byte_E28189;
        //    DebugCrash("Bad resolve zones\t.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    int equivSrcZone = zoneEquivalency[srcZone];
    int equivTargetZone = zoneEquivalency[targetZone];

    if ((equivSrcZone >= sizeOfZE || equivTargetZone >= sizeOfZE)) {
        // if (!byte_E2818A) {
        //    TheCurrentAllowCrashPtr = &byte_E2818A;
        //    DebugCrash("Bad resolve zones\t.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    if (equivTargetZone < equivSrcZone) {
        new_zone = zoneEquivalency[equivTargetZone];
    } else {
        new_zone = zoneEquivalency[equivSrcZone];
    }

    if (new_zone >= sizeOfZE) {
        // if (!byte_E2818B) {
        //    TheCurrentAllowCrashPtr = &byte_E2818B;
        //    DebugCrash("Bad resolve zones\t.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    for (int i = 0; i < sizeOfZE; ++i) {
        unsigned short zone = zoneEquivalency[i];
        if (zone == equivTargetZone || zone == equivSrcZone) {
            zoneEquivalency[i] = new_zone;
        }
    }
}

void Flatten_Zones(unsigned short *sourceZoneEquivalency, unsigned short *targetZoneEquivalency, int sizeOfZE)
{
    for (int i = 0; i < sizeOfZE; ++i) {
        sourceZoneEquivalency[i] =
            targetZoneEquivalency[sourceZoneEquivalency[targetZoneEquivalency[sourceZoneEquivalency[i]]]];
    }

    for (int i = 0; i < sizeOfZE; ++i) {
        int srcZone = sourceZoneEquivalency[i];
        int targetZone = targetZoneEquivalency[i];
        if (srcZone != targetZone) {
            Resolve_Zones(srcZone, targetZone, sourceZoneEquivalency, sizeOfZE);
        }
    }
}
void Apply_Zone(PathfindCell *targetCell, PathfindCell *sourceCell, unsigned short *zoneEquivalency, int sizeOfZE)
{
    if (sourceCell->Get_Temp_Zone() == 0) {
        // if (!byte_E281FC) {
        //    TheCurrentAllowCrashPtr = &byte_E281FC;
        //    DebugCrash("Unset source zone.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    unsigned short srcZone = zoneEquivalency[sourceCell->Get_Temp_Zone()];
    unsigned short targetZone = zoneEquivalency[targetCell->Get_Temp_Zone()];
    if (targetZone == 0) {
        targetCell->Set_Temp_Zone(srcZone);
    } else {
        if (targetZone != srcZone) {
            Resolve_Zones(srcZone, targetZone, zoneEquivalency, sizeOfZE);
        }
    }
}

void PathfindZoneManager::Calculate_Zones(PathfindCell **map, PathfindLayer *layers, const IRegion2D &bounds)
{
    LARGE_INTEGER Frequency;
    LARGE_INTEGER PerformanceCount;

    QueryPerformanceFrequency(&Frequency);
    QueryPerformanceCounter(&PerformanceCount);

    m_maxZone = 1;

    unsigned short zoneEquivalency[24000];

    for (unsigned short i = 0; i < 24000; ++i) {
        zoneEquivalency[i] = i;
    }

    for (int i = 0; i <= LAYER_WALLS; ++i) {
        layers[i].Set_Zone(0);
    }

    int x_count = (bounds.hi.x - bounds.lo.x + 10) / 10;
    int y_count = (bounds.hi.y - bounds.lo.y + 10) / 10;

    for (int x = 0; x < x_count; ++x) {
        for (int y = 0; y < y_count; ++y) {
            IRegion2D region;
            region.lo.x = 10 * x + bounds.lo.x;
            region.lo.y = 10 * y + bounds.lo.y;
            region.hi.x = region.lo.x + 9;
            region.hi.y = region.lo.y + 9;

            if (region.lo.x + 9 > bounds.hi.x) {
                region.hi.x = bounds.hi.x;
            }
            if (region.hi.y > bounds.hi.y) {
                region.hi.y = bounds.hi.y;
            }

            m_visibleZoneBlocks[x][y].Set_Bridge(false);

            for (int ry = region.lo.y; ry <= region.hi.y; ++ry) {
                for (int rx = region.lo.x; rx <= region.hi.x; ++rx) {

                    PathfindCell *region_cell = &map[rx][ry];

                    region_cell->Set_Temp_Zone(0);

                    if (rx > region.lo.x) {
                        if (map[rx][ry].Get_Type() == map[rx - 1][ry].Get_Type()) {
                            Apply_Zone(&map[rx][ry], &map[rx - 1][ry], zoneEquivalency, m_maxZone);
                        }
                    }

                    if (ry > region.lo.y) {
                        if (map[rx][ry].Get_Type() == map[rx][ry - 1].Get_Type()) {
                            Apply_Zone(&map[rx][ry], &map[rx][ry - 1], zoneEquivalency, m_maxZone);
                        }
                    }

                    if (!region_cell->Get_Temp_Zone()) {
                        region_cell->Set_Temp_Zone(m_maxZone);
                        ++m_maxZone;
                    }

                    // INVESIGATE, prob should also check if not wall....
                    if (region_cell->Get_Connect_Layer() > LAYER_GROUND) {
                        m_visibleZoneBlocks[x][y].Set_Bridge(true);
                    }
                }
            }
        }
    }

    int old_max_zone = m_maxZone;

    m_maxZone = 1;

    int collapsedZones[24000];

    collapsedZones[0] = 0;

    for (int i = 1; i < old_max_zone; ++i) {
        int zone = zoneEquivalency[i];
        if (zone == i) {
            collapsedZones[i] = m_maxZone++;
        } else {
            collapsedZones[i] = collapsedZones[zone];
        }
    }

    for (int ry = bounds.lo.y; ry <= bounds.hi.y; ++ry) {
        for (int rx = bounds.lo.x; rx <= bounds.hi.x; ++rx) {
            PathfindCell *region_cell = &map[rx][ry];
            region_cell->Set_Temp_Zone(collapsedZones[region_cell->Get_Temp_Zone()]);
        }
    }

    for (int i = 0; i <= LAYER_WALLS; ++i) {
        PathfindLayer *layer = &layers[i];
        int zone = collapsedZones[layer->Get_Zone()];

        if (zone == 0) {
            zone = m_maxZone++;
        }

        layer->Set_Zone(zone);
        layer->Apply_Zone();

        if (!layer->Is_Unused() && !layer->Is_Destroyed()) {
            ICoord2D cellxy;
            layer->Get_UnkCell1(cellxy);
            Set_Bridge(cellxy.x, cellxy.y, true);

            layer->Get_UnkCell2(cellxy);
            Set_Bridge(cellxy.x, cellxy.y, true);
        }
    }

    Allocate_Zones();

    for (int x = 0; x < x_count; ++x) {
        for (int y = 0; y < y_count; ++y) {
            IRegion2D block_bounds;
            block_bounds.lo.x = 10 * x + bounds.lo.x;
            block_bounds.lo.y = 10 * y + bounds.lo.y;
            block_bounds.hi.x = block_bounds.lo.x + 9;
            block_bounds.hi.y = block_bounds.lo.y + 9;

            if (block_bounds.lo.x + 9 > bounds.hi.x) {
                block_bounds.hi.x = bounds.hi.x;
            }
            if (block_bounds.hi.y > bounds.hi.y) {
                block_bounds.hi.y = bounds.hi.y;
            }

            m_visibleZoneBlocks[x][y].Block_Calculate_Zones(map, layers, block_bounds);
        }
    }

    for (unsigned short i = 0; i < m_zoneTableSize; ++i) {
        m_terrainZones[i] = i;
        m_groundZonesUnknown2[i] = i;
        m_groundZonesUnknown1[i] = i;
        m_groundRubbleZones[i] = i;
        m_groundWaterZones[i] = i;
        m_groundCliffZones[i] = i;
    }

    for (int y = bounds.lo.y; y <= bounds.hi.y; ++y) {
        for (int x = bounds.lo.x; x <= bounds.hi.x; ++x) {
            PathfindCell *target_cell = &map[x][y];

            if (target_cell->Get_Connect_Layer() > 1 && target_cell->Get_Type() == PathfindCell::CELL_CLEAR) {
                PathfindLayer *target_layer = &layers[target_cell->Get_Connect_Layer()];
                int target_zone = target_layer->Get_Zone();
                int source_zone = target_cell->Get_Temp_Zone();
                Resolve_Zones(source_zone, target_zone, m_terrainZones, m_maxZone);
            }

            if (x > bounds.lo.x) {

                if (target_cell->Get_Temp_Zone() != map[x - 1][y].Get_Temp_Zone()) {
                    PathfindCell *x_source_cell = &map[x - 1][y];

                    if (target_cell->Get_Type() == x_source_cell->Get_Type()) {
                        Apply_Zone(target_cell, x_source_cell, m_terrainZones, m_maxZone);
                    } else {
                        bool bool1 = true;

                        if (PathfindCellCompareType4(target_cell, x_source_cell)) {
                            Apply_Zone(target_cell, x_source_cell, m_groundZonesUnknown1, m_maxZone);
                            bool1 = false;
                        }

                        if (PathfindCellCompareType4_Flag(target_cell, x_source_cell)) {
                            Apply_Zone(target_cell, x_source_cell, m_groundZonesUnknown2, m_maxZone);
                            bool1 = false;
                        }
                        if (bool1) {
                            if (PathfindCellCompareType1(target_cell, x_source_cell)) {
                                Apply_Zone(target_cell, x_source_cell, m_groundWaterZones, m_maxZone);
                            } else if (PathfindCellCompareType3(target_cell, x_source_cell)) {
                                Apply_Zone(target_cell, x_source_cell, m_groundRubbleZones, m_maxZone);
                            } else if (PathfindCellCompareType2(target_cell, x_source_cell)) {
                                Apply_Zone(target_cell, x_source_cell, m_groundCliffZones, m_maxZone);
                            }
                        }
                    }
                }
            }
            if (y > bounds.lo.y) {

                if (target_cell->Get_Temp_Zone() != map[x][y - 1].Get_Temp_Zone()) {
                    PathfindCell *y_source_cell = &map[x][y - 1];

                    if (target_cell->Get_Type() == y_source_cell->Get_Type()) {
                        Apply_Zone(target_cell, y_source_cell, m_terrainZones, m_maxZone);
                    } else {
                        bool bool2 = true;

                        if (PathfindCellCompareType4(target_cell, y_source_cell)) {
                            Apply_Zone(target_cell, y_source_cell, m_groundZonesUnknown1, m_maxZone);
                            bool2 = false;
                        }

                        if (PathfindCellCompareType4_Flag(target_cell, y_source_cell)) {
                            Apply_Zone(target_cell, y_source_cell, m_groundZonesUnknown2, m_maxZone);
                            bool2 = false;
                        }

                        if (PathfindCellCompareType1(target_cell, y_source_cell)) {
                            Apply_Zone(target_cell, y_source_cell, m_groundWaterZones, m_maxZone);
                        } else if (PathfindCellCompareType3(target_cell, y_source_cell)) {
                            Apply_Zone(target_cell, y_source_cell, m_groundRubbleZones, m_maxZone);
                        } else if (PathfindCellCompareType2(target_cell, y_source_cell)) {
                            Apply_Zone(target_cell, y_source_cell, m_groundCliffZones, m_maxZone);
                        }
                    }
                }
            }
        }
    }

    for (int i = 1; i < m_maxZone; ++i) {
        m_terrainZones[i] = m_terrainZones[m_terrainZones[i]];
    }

    Flatten_Zones(m_groundCliffZones, m_terrainZones, m_maxZone);
    Flatten_Zones(m_groundWaterZones, m_terrainZones, m_maxZone);
    Flatten_Zones(m_groundRubbleZones, m_terrainZones, m_maxZone);
    Flatten_Zones(m_groundZonesUnknown1, m_terrainZones, m_maxZone);
    Flatten_Zones(m_groundZonesUnknown2, m_terrainZones, m_maxZone);

#if 1
    LARGE_INTEGER __endTime64_1;
    QueryPerformanceCounter(&__endTime64_1);

    static double zonecalc_dbl_E28198;
    static int zonecalccount;
    static double average_time_to_calc_zones;

    zonecalc_dbl_E28198 = (double)(__endTime64_1.QuadPart - PerformanceCount.QuadPart) / (double)Frequency.QuadPart;
    if (zonecalccount >= 400) {
        if (zonecalccount == 400) {
            captainslog_debug(
                " =============DONE============= Average time to calculate zones: %f, \n", average_time_to_calc_zones);
            captainslog_debug("                                           Percent of baseline : %f, \n",
                average_time_to_calc_zones / 0.003335000015795231);
            zonecalccount = 777;
        }
    } else {
        average_time_to_calc_zones =
            ((double)zonecalccount * average_time_to_calc_zones + zonecalc_dbl_E28198) / ((double)zonecalccount + 1.0);
        ++zonecalccount;
        captainslog_debug("computing...: %f, \n", average_time_to_calc_zones);
    }

    if (g_theWriteableGlobalData->m_debugAI == AI_DEBUG_LEVEL_5) {
        float col[3];
        RGBColor color;
        Coord3D pos;

        memset(col, 0, 4u);
        color.red = col[0];
        color.green = col[1];
        color.blue = col[2];

        Add_Icon(nullptr, 0.0, 0, color);

        for (int y = 0; y < bounds.hi.y; ++y) {
            for (int x = 0; x < bounds.hi.x; ++x) {

                int zone = m_terrainZones[map[x][y].Get_Temp_Zone()];

                col[0] = (double)(zone / 3 / 3 % 3) * 0.5;
                col[1] = (double)(zone / 3 % 3) * 0.5;
                col[2] = (double)(zone % 3) * 0.5;

                pos.x = ((double)x + 0.5) * 10.0;
                pos.y = ((double)y + 0.5) * 10.0;

                PathfindLayerEnum layer = map[x][y].Get_Unknown_Layer();
                pos.z = g_theTerrainLogic->Get_Layer_Height(pos.x, pos.y, layer, nullptr, true) + 0.5;
                color.red = col[0];
                color.green = col[1];
                color.blue = col[2];

                Add_Icon(&pos, 8.0, 500, color);
            }
        }
    }
#endif
    m_frameToUpdateZones = -1;
}

void PathfindZoneManager::Reset()
{
    Free_Zones();
    Free_Blocks();
}

void PathfindZoneManager::Set_All_Passable()
{
    for (int x = 0; x < m_zoneBlockExtent.x; ++x) {
        for (int y = 0; y < m_zoneBlockExtent.y; ++y) {
            m_visibleZoneBlocks[x][y].Set_Passable(true);
        }
    }
}
void PathfindZoneManager::Set_Bridge(int cellx, int celly, bool state)
{
    int x = cellx / 0xA;
    int y = celly / 0xA;

    if (x >= 0 && x < m_zoneBlockExtent.x && y >= 0 && y < m_zoneBlockExtent.y) {
        m_visibleZoneBlocks[x][y].Set_Bridge(state);
    }
}

void PathfindZoneManager::Set_Passable(int cellx, int celly, bool state)
{
    int x = cellx / 0xA;
    int y = celly / 0xA;

    if (x < 0 || x >= m_zoneBlockExtent.x) {
        // if (!byte_E281AC) {
        //    TheCurrentAllowCrashPtr = &byte_E281AC;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    } else if (y < 0 || y >= m_zoneBlockExtent.y) {
        // if (!byte_E281AD) {
        //    TheCurrentAllowCrashPtr = &byte_E281AD;
        //    DebugCrash("Invalid block.");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    } else {
        m_visibleZoneBlocks[x][y].Set_Passable(state);
    }
}

PathfindZoneManager::~PathfindZoneManager()
{
    Free_Zones();
    Free_Blocks();
}

void PathfindZoneManager::Get_Zone_Block_Extent(ICoord2D *extent)
{
    extent->x = m_zoneBlockExtent.x;
    extent->y = m_zoneBlockExtent.y;
}

bool PathfindZoneManager::Is_To_Update_Zones()
{
    return g_theGameLogic->Get_Frame() >= m_frameToUpdateZones;
}