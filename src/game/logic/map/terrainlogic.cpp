/**
 * @file
 *
 * @author Jonathan Wilson
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
#include "terrainlogic.h"

#include "gamelogic.h"
#include "object.h"
#include "plane.h"
#include "thingfactory.h"
#include "tri.h"

#ifndef GAME_DLL
TerrainLogic *g_theTerrainLogic = nullptr;
#endif

// uses modified Cohen-Sutherland line clipping algorithm
bool Line_In_Region(const Coord2D *p1, const Coord2D *p2, const Region2D *region)
{
    enum OutCodeEnum
    {
        CODE_INSIDE = 0, // 0000
        CODE_LEFT = 1, // 0001
        CODE_RIGHT = 2, // 0010
        CODE_BOTTOM = 4, // 0100
        CODE_TOP = 8, // 1000
    };

    float x_min = region->lo.x;
    float x_max = region->hi.x;
    float y_min = region->lo.y;
    float y_max = region->hi.y;

    float x0 = p1->x;
    float y0 = p1->y;
    float x1 = p2->x;
    float y1 = p2->y;

    /**
     *  Compute outcodes for P0, P1, and whatever point lies outside the clip region.
     */
    int out_code0 = CODE_INSIDE;

    if (x0 < x_min) {
        out_code0 |= CODE_LEFT;
    } else if (x0 > x_max) {
        out_code0 |= CODE_RIGHT;
    }

    if (y0 < y_min) {
        out_code0 |= CODE_TOP;
    } else if (y0 > y_max) {
        out_code0 |= CODE_BOTTOM;
    }

    int out_code1 = CODE_INSIDE;

    if (x1 < x_min) {
        out_code1 |= CODE_LEFT;
    } else if (x1 > x_max) {
        out_code1 |= CODE_RIGHT;
    }

    if (y1 < y_min) {
        out_code1 |= CODE_TOP;
    } else if (y1 > y_max) {
        out_code1 |= CODE_BOTTOM;
    }

    if (!(out_code0 | out_code1)) {
        /**
         *  bitwise OR is 0: Both points inside window; trivially accept and return true.
         */
        return true;
    }

    if (out_code0 & out_code1) {
        /**
         *  Bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
         *  or BOTTOM), so both must be outside region;
         */
        return false;
    }

    float delta;

    if (out_code0 != CODE_INSIDE) {
        if (out_code0 & CODE_TOP) {
            delta = y1 - y0;

            if (delta == 0.0f) {
                return false;
            }

            x0 = (x1 - x0) * (y_min - y0) / delta + x0;
            y0 = y_min;

        } else if (out_code0 & CODE_BOTTOM) {
            delta = y1 - y0;

            if (delta == 0.0f) {
                return false;
            }

            x0 = (x1 - x0) * (y_max - y0) / delta + x0;
            y0 = y_max;
        }

        // TODO examine. Doesn't check code.. bug?
        if (x0 > x_max) { // CODE_RIGHT
            delta = x1 - x0;

            if (delta == 0.0f) {
                return false;
            }

            y0 = (y1 - y0) * (x_max - x0) / delta + y0;
            x0 = x_max;

        } else if (x0 < x_min) { // CODE_LEFT
            delta = x1 - x0;

            if (delta == 0.0f) {
                return false;
            }

            y0 = (y1 - y0) * (x_min - x0) / delta + y0;
            x0 = x_min;
        }
    }

    if (out_code1 != CODE_INSIDE) {
        if (out_code1 & CODE_TOP) {
            delta = y1 - y0;

            if (delta == 0.0f) {
                return false;
            }

            x1 = (x1 - x0) * (y_min - y1) / delta + x1;
            y1 = y_min;

        } else if (out_code1 & CODE_BOTTOM) {
            delta = y1 - y0;

            if (delta == 0.0f) {
                return false;
            }

            x1 = (x1 - x0) * (y_max - y1) / delta + x1;
            y1 = y_max;
        }

        // TODO examine. Doesn't check code.. bug?
        if (x1 > x_max) { // CODE_RIGHT
            delta = x1 - x0;

            if (delta == 0.0f) {
                return false;
            }

            y1 = (y1 - y0) * (x_max - x1) / delta + y1;
            x1 = x_max;

        } else if (x1 < x_min) { // CODE_LEFT
            delta = x1 - x0;

            if (delta == 0.0f) {
                return false;
            }

            y1 = (y1 - y0) * (x_min - x1) / delta + y1;
            x1 = x_min;
        }
    }

    return x0 >= x_min && x0 <= x_max && y0 >= y_min && y0 <= y_max && x1 >= x_min && x1 <= x_max && y1 >= y_min
        && y1 <= y_max;
}

BridgeInfo::BridgeInfo() :
    bridge_width(0.0f),
    bridge_index(0),
    cur_damage_state(BODY_PRISTINE),
    damage_state_changed(false),
    bridge_object_id(OBJECT_UNK)
{
    for (int i = 0; i < BRIDGE_MAX_TOWERS; i++) {
        tower_object_id[i] = OBJECT_UNK;
    }
}

// TODO no idea how to do OBJECT_STATUS_MASK_NONE
#if 0
Bridge::Bridge(BridgeInfo &info, Dict *props, Utf8String bridge_template_name) :
    // BUGFIX, init layer
    m_layer(LAYER_UNK)
{
    m_bridgeInfo = info;
    m_templateName = bridge_template_name;

    m_bounds.lo.x = m_bridgeInfo.from_left.x;
    m_bounds.lo.y = m_bridgeInfo.from_left.y;

    m_bounds.hi.x = m_bounds.lo.x;
    m_bounds.hi.y = m_bounds.lo.y;

    if (m_bounds.lo.x > m_bridgeInfo.from_right.x) {
        m_bounds.lo.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.from_right.y) {
        m_bounds.lo.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.from_right.x) {
        m_bounds.hi.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.from_right.y) {
        m_bounds.hi.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_left.x) {
        m_bounds.lo.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_left.y) {
        m_bounds.lo.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_left.x) {
        m_bounds.hi.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_left.y) {
        m_bounds.hi.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_right.x) {
        m_bounds.lo.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_right.y) {
        m_bounds.lo.y = m_bridgeInfo.to_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_right.x) {
        m_bounds.hi.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_right.y) {
        m_bounds.hi.y = m_bridgeInfo.to_right.y;
    }

    m_bridgeInfo.cur_damage_state = BODY_PRISTINE;

    static const ThingTemplate *_generic_bridge_template = g_theThingFactory->Find_Template("GenericBridge", true);

    if (_generic_bridge_template == nullptr) {
        captainslog_debug("*** GenericBridge template not found.");
        return;
    }

    Object *obj = g_theThingFactory->New_Object(
        _generic_bridge_template, nullptr, OBJECT_STATUS_MASK_NONE[0], OBJECT_STATUS_MASK_NONE[1]);

    Coord3D pos;
    pos.x = (m_bridgeInfo.from_left.x + m_bridgeInfo.to_right.x) / 2.0f;
    pos.y = (m_bridgeInfo.from_left.y + m_bridgeInfo.to_right.y) / 2.0f;
    pos.z = (m_bridgeInfo.from_left.z + m_bridgeInfo.to_right.z) / 2.0f;

    obj->Set_Position(&pos);

    m_bridgeInfo.bridge_object_id = obj->Get_ID();

    obj->Update_Obj_Values_From_Map_Properties(props);

    Coord2D xy;
    xy.x = m_bridgeInfo.to_left.x - m_bridgeInfo.from_left.x;
    xy.y = m_bridgeInfo.to_left.y - m_bridgeInfo.from_left.y;

    obj->Set_Orientation(xy.To_Angle());

    xy.x = m_bridgeInfo.to_left.x - m_bridgeInfo.to_right.x;
    xy.y = m_bridgeInfo.to_left.y - m_bridgeInfo.to_right.y;

    // TODO examine, possible bug or copy pasted code here nothing uses this...
    xy.Normalize();

    TerrainRoadType *bridge = g_theTerrainRoads->Find_Bridge(m_templateName);

    if (!bridge) {
        captainslog_debug("*** Bridge Template Not Found '%s'.", m_templateName.Str());
    } else {
        m_next = nullptr;
    }
}
#endif

Bridge::Bridge(Object *obj) :
    // BUGFIX, init layer
    m_layer(LAYER_UNK)
{
    const ThingTemplate *templ = obj->Get_Template();

    m_templateName = templ->Get_Name();

    const GeometryInfo &geoinfo = obj->Get_Geometry_Info();

    if (geoinfo.Get_Type() != GEOMETRY_BOX) {
        captainslog_debug("Bridges need to be rectangles.\n");
    }

    const Coord3D *obj_pos = obj->Get_Position();

    float orientation = obj->Get_Orientation();

    float maj_rad = geoinfo.Get_Major_Radius();
    float min_rad = geoinfo.Get_Minor_Radius();

    m_bridgeInfo.bridge_width = 2.0f * min_rad;

    float cosval = GameMath::Cos(orientation);
    float sinval = GameMath::Sin(orientation);

    float fl_y = obj_pos->y + min_rad * cosval - maj_rad * sinval;
    float fl_x = obj_pos->x - maj_rad * cosval - min_rad * sinval;
    m_bridgeInfo.from_left.Set(fl_x, fl_y, obj_pos->z);

    float tl_y = obj_pos->y + min_rad * cosval + maj_rad * sinval;
    float tl_x = obj_pos->x + maj_rad * cosval - min_rad * sinval;
    m_bridgeInfo.to_left.Set(tl_x, tl_y, obj_pos->z);

    float fr_y = obj_pos->y - min_rad * cosval - maj_rad * sinval;
    float fr_x = obj_pos->x - maj_rad * cosval + min_rad * sinval;
    m_bridgeInfo.from_right.Set(fr_x, fr_y, obj_pos->z);

    float tr_y = obj_pos->y - min_rad * cosval + maj_rad * sinval;
    float tr_x = obj_pos->x + maj_rad * cosval + min_rad * sinval;
    m_bridgeInfo.to_right.Set(tr_x, tr_y, obj_pos->z);

    m_bridgeInfo.from.x = (m_bridgeInfo.from_left.x + m_bridgeInfo.from_right.x) / 2.0f;
    m_bridgeInfo.from.y = (m_bridgeInfo.from_left.y + m_bridgeInfo.from_right.y) / 2.0f;
    m_bridgeInfo.from.z = (m_bridgeInfo.from_left.z + m_bridgeInfo.from_right.z) / 2.0f;

    m_bridgeInfo.to.x = (m_bridgeInfo.to_left.x + m_bridgeInfo.to_right.x) / 2.0f;
    m_bridgeInfo.to.y = (m_bridgeInfo.to_left.y + m_bridgeInfo.to_right.y) / 2.0f;
    m_bridgeInfo.to.z = (m_bridgeInfo.to_left.z + m_bridgeInfo.to_right.z) / 2.0f;

    m_bounds.lo.x = m_bridgeInfo.from_left.x;
    m_bounds.lo.y = m_bridgeInfo.from_left.y;

    m_bounds.hi.x = m_bounds.lo.x;
    m_bounds.hi.y = m_bounds.lo.y;

    if (m_bounds.lo.x > m_bridgeInfo.from_right.x) {
        m_bounds.lo.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.from_right.y) {
        m_bounds.lo.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.from_right.x) {
        m_bounds.hi.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.from_right.y) {
        m_bounds.hi.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_left.x) {
        m_bounds.lo.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_left.y) {
        m_bounds.lo.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_left.x) {
        m_bounds.hi.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_left.y) {
        m_bounds.hi.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_right.x) {
        m_bounds.lo.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_right.y) {
        m_bounds.lo.y = m_bridgeInfo.to_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_right.x) {
        m_bounds.hi.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_right.y) {
        m_bounds.hi.y = m_bridgeInfo.to_right.y;
    }

    m_bridgeInfo.cur_damage_state = BODY_PRISTINE;
    m_bridgeInfo.bridge_object_id = obj->Get_ID();

    TerrainRoadType *bridge = g_theTerrainRoads->Find_Bridge(m_templateName);

    if (bridge == nullptr) {
        captainslog_debug("*** Bridge Template Not Found '%s'.", m_templateName.Str());
        return;
    }

    Coord2D to;

    to.x = m_bridgeInfo.to_left.x - m_bridgeInfo.to_right.x;
    to.y = m_bridgeInfo.to_left.y - m_bridgeInfo.to_right.y;

    to.Normalize();

    Coord3D coords[BRIDGE_MAX_TOWERS];
    coords[BRIDGE_TOWER_FROM_LEFT] = m_bridgeInfo.from_left;
    coords[BRIDGE_TOWER_FROM_RIGHT] = m_bridgeInfo.from_right;
    coords[BRIDGE_TOWER_TO_LEFT] = m_bridgeInfo.to_left;
    coords[BRIDGE_TOWER_TO_RIGHT] = m_bridgeInfo.to_right;

    float tower_major_radius = 5.0;

    for (int type = 0; type < BRIDGE_MAX_TOWERS; ++type) {

        Utf8String tower_obj_name = bridge->Get_Tower_Object_Name(BridgeTowerType(type));

        ThingTemplate const *tower_template = g_theThingFactory->Find_Template(tower_obj_name, true);

        if (tower_template != nullptr) {
            tower_major_radius = tower_template->Get_Template_Geometry_Info().Get_Major_Radius();
        }

        Coord3D worldPos = coords[type];

        switch (type) {
            case BRIDGE_TOWER_FROM_LEFT:
            case BRIDGE_TOWER_TO_LEFT:
                worldPos.x = to.x * tower_major_radius + worldPos.x;
                worldPos.y = to.y * tower_major_radius + worldPos.y;
                break;
            case BRIDGE_TOWER_FROM_RIGHT:
            case BRIDGE_TOWER_TO_RIGHT:
                worldPos.x = worldPos.x - to.x * tower_major_radius;
                worldPos.y = worldPos.y - to.y * tower_major_radius;
                break;
            default:
                break;
        }

        Object *tower_obj = Create_Tower(&worldPos, BridgeTowerType(type), tower_template, obj);

        if (tower_obj != nullptr) {
            m_bridgeInfo.tower_object_id[type] = tower_obj->Get_ID();
        }
    }

    m_next = nullptr;
}

Object *Bridge::Create_Tower(
    Coord3D *world_pos, BridgeTowerType tower_type, const ThingTemplate *tower_template, Object *bridge)
{
    // TODO needs bits of Object, Pathfinder, AI.
#ifdef GAME_DLL
    return Call_Method<Object *, Bridge, Coord3D *, BridgeTowerType, const ThingTemplate *, Object *>(
        PICK_ADDRESS(0x004479B0, 0x0074127F), this, world_pos, tower_type, tower_template, bridge);
#else
    return nullptr;
#endif
}

float Bridge::Get_Bridge_Height(const Coord3D *loc, Coord3D *n) const
{
    Vector3 from_left(m_bridgeInfo.from_left.x, m_bridgeInfo.from_left.y, m_bridgeInfo.from_left.z);
    Vector3 from_right(m_bridgeInfo.from_right.x, m_bridgeInfo.from_right.y, m_bridgeInfo.from_right.z);
    Vector3 to_left(m_bridgeInfo.to_left.x, m_bridgeInfo.to_left.y, m_bridgeInfo.to_left.z);

    PlaneClass plane(from_left, from_right, to_left);

    float z = 1000.0f;
    Vector3 p0(loc->x, loc->y, 0.0f);
    Vector3 p1(loc->x, loc->y, z);

    float set_t;
    plane.Compute_Intersection(p0, p1, &set_t);

    if (n) {
        n->x = plane.N.X;
        n->y = plane.N.Y;
        n->z = plane.N.Z;
    }

    return set_t * z;
}

bool Bridge::Is_Cell_Entry_Point(const Region2D *cell) const
{
    Coord3D coord1 = m_bridgeInfo.from_right - m_bridgeInfo.from_left;

    coord1.Normalize();

    coord1.x *= 10.0;
    coord1.y *= 10.0;

    Coord3D coord2 = m_bridgeInfo.to - m_bridgeInfo.from;

    coord2.Normalize();

    coord2.x *= 5.0;
    coord2.y *= 5.0;

    Coord3D coord3 = m_bridgeInfo.from_left;
    coord3.x -= coord2.x;
    coord3.y -= coord2.y;
    coord3.x += coord1.x;
    coord3.y += coord1.y;

    Coord3D coord4 = m_bridgeInfo.from_right;
    coord4.x -= coord2.x;
    coord4.y -= coord2.y;
    coord4.x -= coord1.x;
    coord4.y -= coord1.y;

    Coord3D coord5 = m_bridgeInfo.to_left;
    coord5.x += coord2.x;
    coord5.y += coord2.y;
    coord5.x += coord1.x;
    coord5.y += coord1.y;

    Coord3D coord6 = m_bridgeInfo.to_right;
    coord6.x += coord2.x;
    coord6.y += coord2.y;
    coord6.x -= coord1.x;
    coord6.y -= coord1.y;

    Coord2D pt1;
    Coord2D pt2;

    pt1.x = coord3.x;
    pt1.y = coord3.y;
    pt2.x = coord4.x;
    pt2.y = coord4.y;

    if (Line_In_Region(&pt1, &pt2, cell)) {
        return true;
    }

    pt1.x = coord5.x;
    pt1.y = coord5.y;
    pt2.x = coord6.x;
    pt2.y = coord6.y;
    return Line_In_Region(&pt1, &pt2, cell) != false;
}

bool Bridge::Is_Cell_On_End(const Region2D *cell) const
{
    Coord3D coord1 = m_bridgeInfo.from_right - m_bridgeInfo.from_left;

    coord1.Normalize();

    coord1.x *= 10.0f;
    coord1.y *= 10.0f;

    Coord3D coord2 = m_bridgeInfo.from_left;
    coord2.x += coord1.x;
    coord2.y += coord1.y;

    Coord3D coord3 = m_bridgeInfo.from_right;
    coord3.x -= coord1.x;
    coord3.y -= coord1.y;

    Coord3D coord4 = m_bridgeInfo.to_left;
    coord4.x += coord1.x;
    coord4.y += coord1.y;

    Coord3D coord5 = m_bridgeInfo.to_right;
    coord5.x -= coord1.x;
    coord5.y -= coord1.y;

    Coord2D pt1;
    Coord2D pt2;

    pt1.x = coord2.x;
    pt1.y = coord2.y;

    pt2.x = coord3.x;
    pt2.y = coord3.y;

    if (Line_In_Region(&pt1, &pt2, cell)) {
        return true;
    }

    pt1.x = coord4.x;
    pt1.y = coord4.y;

    pt2.x = coord5.x;
    pt2.y = coord5.y;

    return Line_In_Region(&pt1, &pt2, cell) != false;
}

bool Bridge::Is_Cell_On_Side(const Region2D *cell) const
{
    Coord3D coord1 = m_bridgeInfo.from_right - m_bridgeInfo.from_left;

    coord1.Normalize();

    coord1.x *= 5.0999999;
    coord1.y *= 5.0999999;

    Coord3D coord2 = m_bridgeInfo.from_left;
    coord2.x -= coord1.x;
    coord2.y -= coord1.y;

    Coord3D coord3 = m_bridgeInfo.from_right;
    coord3.x += coord1.x;
    coord3.y += coord1.y;

    Coord3D coord4 = m_bridgeInfo.to_left;
    coord4.x -= coord1.x;
    coord4.y -= coord1.y;

    Coord3D coord5 = m_bridgeInfo.to_right;
    coord5.x += coord1.x;
    coord5.y += coord1.y;

    Coord2D pt1;
    Coord2D pt2;

    pt1.x = coord2.x;
    pt1.y = coord2.y;
    pt2.x = coord4.x;
    pt2.y = coord4.y;

    if (Line_In_Region(&pt1, &pt2, cell)) {
        return true;
    }

    pt1.x = coord3.x;
    pt1.y = coord3.y;
    pt2.x = coord5.x;
    pt2.y = coord5.y;

    if (Line_In_Region(&pt1, &pt2, cell)) {
        return true;
    }

    coord2.x -= coord1.x;
    coord2.y -= coord1.y;

    coord3.x += coord1.x;
    coord3.y += coord1.y;

    coord4.x -= coord1.x;
    coord4.y -= coord1.y;

    Coord3D coord6;
    coord6.x = coord5.x + coord1.x;
    coord6.y = coord5.y + coord1.y;

    pt1.x = coord2.x;
    pt1.y = coord2.y;
    pt2.x = coord4.x;
    pt2.y = coord4.y;

    if (Line_In_Region(&pt1, &pt2, cell)) {
        return true;
    }

    pt1.x = coord3.x;
    pt1.y = coord3.y;
    pt2.x = coord6.x;
    pt2.y = coord6.y;
    return Line_In_Region(&pt1, &pt2, cell) != false;
}

bool Bridge::Is_Point_On_Bridge(const Coord3D *loc) const
{
    unsigned char flags;

    if (loc->x < m_bounds.lo.x) {
        return false;
    }
    if (loc->x > m_bounds.hi.x) {
        return false;
    }
    if (loc->y < m_bounds.lo.y) {
        return false;
    }
    if (loc->y > m_bounds.hi.y) {
        return false;
    }

    Vector3 test(loc->x, loc->y, loc->z);

    Vector3 from_left(m_bridgeInfo.from_left.x, m_bridgeInfo.from_left.y, m_bridgeInfo.from_left.z);
    Vector3 from_right(m_bridgeInfo.from_right.x, m_bridgeInfo.from_right.y, m_bridgeInfo.from_right.z);
    Vector3 to_left(m_bridgeInfo.to_left.x, m_bridgeInfo.to_left.y, m_bridgeInfo.to_left.z);
    Vector3 to_right(m_bridgeInfo.to_right.x, m_bridgeInfo.to_right.y, m_bridgeInfo.to_right.z);

    if (Point_In_Triangle_2D(from_left, from_right, to_left, test, 0, 1, flags)) {
        return true;
    }

    return Point_In_Triangle_2D(from_right, to_left, to_right, test, 0, 1, flags) != false;
}

Drawable *Bridge::Pick_Bridge(const Vector3 &from, const Vector3 &to, Vector3 *pos) const
{
    float set_t;

    Vector3 from_left(m_bridgeInfo.from_left.x, m_bridgeInfo.from_left.y, m_bridgeInfo.from_left.z);
    Vector3 from_right(m_bridgeInfo.from_right.x, m_bridgeInfo.from_right.y, m_bridgeInfo.from_right.z);
    Vector3 to_left(m_bridgeInfo.to_left.x, m_bridgeInfo.to_left.y, m_bridgeInfo.to_left.z);

    PlaneClass plane(from_left, from_right, to_left);
    plane.Compute_Intersection(from, to, &set_t);

    Vector3 loc = from + ((to - from) * set_t);

    Coord3D coord(loc.X, loc.Y, loc.Z);

    if (Is_Point_On_Bridge(&coord)) {

        // TODO, look into, possible bug, we shouldn't change position if we don't have a object?
        *pos = loc;

        Object *obj = g_theGameLogic->Find_Object_By_ID(m_bridgeInfo.bridge_object_id);

        if (obj != nullptr) {
            return obj->Get_Drawable();
        }
    }

    return nullptr;
}

void Bridge::Update_Damage_State()
{
    // TODO needs bits of Object, Pathfinder, AI.
#ifdef GAME_DLL
    Call_Method<void, Bridge>(PICK_ADDRESS(0x004497C0, 0x00742D78), this);
#endif
}

Bridge *TerrainLogic::Find_Bridge_At(const Coord3D *loc) const
{
    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {
        if (i->Is_Point_On_Bridge(loc)) {
            return i;
        }
    }

    return nullptr;
}

Bridge *TerrainLogic::Find_Bridge_Layer_At(const Coord3D *loc, PathfindLayerEnum layer, bool b) const
{
    if (layer == PathfindLayerEnum(1)) {
        return nullptr;
    }

    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {
        if (i->Get_Layer() == layer && (!b || i->Is_Point_On_Bridge(loc))) {
            return i;
        }
    }

    return nullptr;
}

bool TerrainLogic::Object_Interacts_With_Bridge_End(Object *obj, int layer) const
{
    if (layer == PathfindLayerEnum(1)) {
        return false;
    }

    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {
        if (i->Get_Layer() == layer) {

            bool on_end = false;

            float radius = obj->Get_Geometry_Info().Get_Minor_Radius() + 5.0f;

            const Coord3D *pos = obj->Get_Position();

            Region2D region;
            region.lo.x = pos->x;
            region.lo.y = pos->y;

            region.hi.x = pos->x;
            region.hi.y = pos->y;

            region.lo.x -= radius;
            region.lo.y -= radius;

            region.hi.x += radius;
            region.hi.y += radius;

            if (i->Is_Cell_On_End(&region)) {
                on_end = true;
            }
            if (on_end) {

                float bridge_height = i->Get_Bridge_Height(pos, nullptr);

                return fabs(pos->z - bridge_height) <= 10.0;
            }
            break;
        }
    }

    return false;
}

Drawable *TerrainLogic::Pick_Bridge(const Vector3 &from, const Vector3 &to, Vector3 *pos)
{
    Drawable *drawable = nullptr;

    Vector3 new_pos(0.0, 0.0, 0.0);

    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {
        Vector3 bridge_pos;
        Drawable *pick = i->Pick_Bridge(from, to, &bridge_pos);
        if (!drawable) {
            drawable = pick;
            new_pos = bridge_pos;
        }
    }

    *pos = new_pos;

    return drawable;
}

void TerrainLogic::Update_Bridge_Damage_States()
{
    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {
        i->Update_Damage_State();
    }

    m_bridgeDamageStatesChanged = true;
}

bool TerrainLogic::Is_Bridge_Repaired(const Object *bridge) const
{
    if (!bridge) {
        return false;
    }

    ObjectID id = bridge->Get_ID();

    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {
        BridgeInfo const *info = i->Peek_Bridge_Info();

        if (info->bridge_object_id == id) {
            return info->damage_state_changed && info->cur_damage_state != BODY_RUBBLE;
        }
    }
    return false;
}

bool TerrainLogic::Is_Bridge_Broken(const Object *bridge) const
{
    if (!bridge) {
        return false;
    }

    ObjectID id = bridge->Get_ID();

    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {
        BridgeInfo const *info = i->Peek_Bridge_Info();

        if (info->bridge_object_id == id) {
            return info->damage_state_changed && info->cur_damage_state == BODY_RUBBLE;
        }
    }

    return false;
}

void TerrainLogic::Get_Bridge_Attack_Points(const Object *bridge, TBridgeAttackInfo *attack_info) const
{
    ObjectID obj_id = bridge->Get_ID();

    for (Bridge *i = Get_First_Bridge(); i != nullptr; i = i->Get_Next()) {

        const BridgeInfo *info = i->Peek_Bridge_Info();
        if (info->bridge_object_id == obj_id) {
            Coord3D pos;
            pos.x = info->to.x - info->from.x;
            pos.y = info->to.y - info->from.y;
            pos.z = info->to.z - info->from.z;

            pos.Normalize();

            Coord3D coord;
            coord.x = info->from_right.x - info->from_left.x;
            coord.y = info->from_right.y - info->from_left.y;
            coord.z = info->from_right.z - info->from_left.z;

            float length = coord.Length() / 2.0f;

            attack_info->attack_point1.x = info->from.x + pos.x * length;
            attack_info->attack_point1.y = info->from.y + pos.y * length;
            attack_info->attack_point1.z = info->from.z + pos.z * length;

            attack_info->attack_point2.x = info->to.x - pos.x * length;
            attack_info->attack_point2.y = info->to.y - pos.y * length;
            attack_info->attack_point2.z = info->to.z - pos.z * length;
            return;
        }
    }

    attack_info->attack_point1 = *bridge->Get_Position();
    attack_info->attack_point2 = *bridge->Get_Position();
}
