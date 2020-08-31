/**
 * @file
 *
 * @author tomsons26
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

#include "always.h"
#include "w3dpoly.h"
#include "plane.h"
#include "vector3.h"

void ClipPolyClass::Reset()
{
    return m_verts.Delete_All();
}

void ClipPolyClass::Add_Vertex(const Vector3 &point)
{
    m_verts.Add(point);
}

void ClipPolyClass::Clip(const PlaneClass &plane, ClipPolyClass &dest)
{
    dest.Reset();

    int vcount = m_verts.Count();

    if (vcount > 2) {
        int j = 0;
        int k = vcount - 1;
        float alpha;

        Vector3 int_point;
        bool prev_point_in_front = plane.In_Front(m_verts[k]); // wb `has == false` here;

        for (int i = 0; i < vcount; ++i) {
            bool point_in_front = plane.In_Front(m_verts[j]); // wb `has == false` here;

            if (prev_point_in_front) {
                if (point_in_front) {
                    dest.Add_Vertex(m_verts[j]);
                } else {
                    plane.Compute_Intersection(m_verts[k], m_verts[j], &alpha);
                    Vector3::Lerp(m_verts[k], m_verts[j], alpha, &int_point);
                    dest.Add_Vertex(int_point);
                }
            } else if (point_in_front) {
                plane.Compute_Intersection(m_verts[k], m_verts[j], &alpha);
                Vector3::Lerp(m_verts[k], m_verts[j], alpha, &int_point);
                dest.Add_Vertex(int_point);
                dest.Add_Vertex(m_verts[j]);
            }
            prev_point_in_front = point_in_front;
			
			//what's going on here?

            // enb has
            // k = j;
            // int t = j + 1;
            // j = t % vcount;

            // wb has
            k = j++;
            if (j >= vcount) {
                j = 0;
            }
        }
    }
}