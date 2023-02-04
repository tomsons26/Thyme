/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Pathfinder Layer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aipathfind.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include "ai.h"
#include "locomotor.h"
#include "pathfinder_cell.h"
#include "terrainlogic.h"

PathfindLayer::PathfindLayer() :
    m_cells(nullptr),
    m_layerCells(nullptr),
    m_width(0),
    m_height(0),
    m_xOrigin(0),
    m_yOrigin(0),
    m_zone(0),
    m_bridge(nullptr),
    m_destroyed(false)
{
    m_unk1.x = -1;
    m_unk1.y = -1;
    m_unk2.x = -1;
    m_unk2.y = -1;
}

PathfindLayer::~PathfindLayer()
{
    Reset();
}

void PathfindLayer::Reset()
{
    m_bridge = nullptr;

    if (m_layerCells != nullptr) {
        for (int x = 0; x < m_width; ++x) {
            for (int y = 0; y < m_height; ++y) {
                m_layerCells[x][y].Reset();
            }
        }

        delete[] m_layerCells;
        m_layerCells =  nullptr;
    }
    if (m_cells != nullptr) {
            delete[] m_cells;
        m_cells = nullptr;
    }

    m_width = 0;
    m_height = 0;

    m_xOrigin = 0;
    m_yOrigin = 0;

    m_unk1.x = -1;
    m_unk1.y = -1;
    m_unk2.x = -1;
    m_unk2.y = -1;

    m_layer = LAYER_GROUND;
}

bool PathfindLayer::Is_Unused()
{
    if (m_layer == LAYER_WALLS && m_width > 0) {
        return false;
    }

    return m_bridge == nullptr;
}

bool PathfindLayer::Init(Bridge *bridge, PathfindLayerEnum layer)
{
    if (m_bridge) {
        return false;
    }

    m_bridge = bridge;
    m_layer = layer;
    m_destroyed = false;

    return true;
}

bool PathfindLayer::Connects_Zones(PathfindZoneManager *manager, LocomotorSet *loco, int zone1, int zone2)
{
    if (!m_destroyed) {
        return false;
    }

    bool connects_zone_1 = false;
    bool connects_zone_2 = false;

    for (int y = 0; y < m_width; ++y) {
        for (int x = 0; x < m_height; ++x) {
            if (m_layerCells[y][x].Get_Connect_Layer() == LAYER_GROUND) {
                int cellx = m_yOrigin + x;
                int celly = m_xOrigin + y;

                PathfindCell *cell = g_theAI->Get_Pathfinder()->Get_Cell(LAYER_GROUND, celly, cellx);

                if (cell == nullptr) {
                    // if (!byte_E281BD) {
                    //    TheCurrentAllowCrashPtr = &byte_E281BD;
                    //    DebugCrash("Should have cell.");
                    //    TheCurrentAllowCrashPtr = 0;
                    //}
                }

                if (cell != nullptr) {
                    unsigned short eff_zone =
                        manager->Get_Effective_Zone(loco->Get_Valid_Surfaces(), true, cell->Get_Temp_Zone());
                    unsigned short eff_ter_zone = manager->Get_Effective_Terrain_Zone(eff_zone);

                    if (eff_ter_zone == zone1) {
                        connects_zone_1 = true;
                    }
                    if (eff_ter_zone == zone2) {
                        connects_zone_2 = true;
                    }
                }
            }
        }
    }
    return connects_zone_1 && connects_zone_2;
}

bool PathfindLayer::Set_Destroyed(bool state)
{
    if (state == m_destroyed) {
        return false;
    }
    m_destroyed = state;
    // Classify_Cells();
    return true;
}

void PathfindLayer::Apply_Zone()
{
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            m_layerCells[x][y].Set_Temp_Zone(m_zone);
        }
    }
}

ObjectID PathfindLayer::Get_Bridge_ID()
{
    return m_bridge->Peek_Bridge_Info()->bridge_object_id;
}

PathfindCell *PathfindLayer::Get_Cell(int x, int y)
{

    if (m_layerCells == nullptr) {
        // if (!byte_E281C3) {
        //    TheCurrentAllowCrashPtr = &byte_E281C3;
        //    DebugCrash("no data in layer, why get cells?");
        //    TheCurrentAllowCrashPtr = 0;
        //}
    }

    if (m_layerCells == nullptr) {
        return nullptr;
    }
    x -= m_xOrigin;
    y -= m_yOrigin;

    if (x < 0 || x >= m_width) {
        return nullptr;
    }

    if (y < 0 || y >= m_height) {
        return nullptr;
    }

    PathfindCell *cell = &m_layerCells[x][y];
    if (cell->Get_Type() == PathfindCell::CELL_TYPE_6) {
        return nullptr;
    }

    return cell;
}
