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
#pragma once

#include "wwstring.h"
#include "matrix3d.h"

class ProxyClass
{
private:
    StringClass m_name;
    Matrix3D m_transform;

public:
    ProxyClass() {}
    ProxyClass(const char *name, class Matrix3D &tm) : m_name(name), m_transform(tm) {}
    const char *Get_Name() { return m_name; }
    void Set_Name(const char *name) { m_name = name; }
    const Matrix3D &Get_Transform() { return m_transform; }
    void Set_Transform(const Matrix3D &tm) { m_transform = tm; }
    bool operator==(const ProxyClass &src) { return (m_name == src.m_name && m_transform == src.m_transform); }
    bool operator!=(const ProxyClass &src) { return (m_name != src.m_name || m_transform != src.m_transform); }
};
