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

class RenderObjClass;
class ChunkLoadClass;

#include "w3dmpo.h"

class PrototypeClass
{
public:
    PrototypeClass() : m_nextHash(nullptr) {}

    virtual ~PrototypeClass() {}
    virtual const char *Get_Name() const = 0;
    virtual int Get_Class_ID() const = 0;
    virtual RenderObjClass *Create() = 0;

private:
    PrototypeClass *m_nextHash;
};

class PrototypeLoaderClass
{
public:
    virtual int Chunk_Type() const = 0;
    virtual PrototypeClass *Load_W3D(ChunkLoadClass &chunk_load) = 0;
};

class PrimitivePrototypeClass : class PrototypeClass, class W3DMPO
{
    IMPLEMENT_W3D_POOL(PrimitivePrototypeClass)
public:
    PrimitivePrototypeClass(RenderObjClass *proto)
    {
        m_Proto = proto;
        reinterpret_cast<RefCountClass*>(m_Proto)->Add_Ref(); //temp until RenderObjClass is in
    }

    ~PrimitivePrototypeClass() { reinterpret_cast<RefCountClass*>(m_Proto)->Release_Ref(); } //temp until RenderObjClass is in

    virtual const char *Get_Name() const { return m_Proto->Get_Name(); }
    virtual int Get_Class_ID() const { return m_Proto->Class_ID(); }
    virtual RenderObjClass *Create() { return m_Proto->Clone(); }
    virtual void Delete_Self() { delete this; }

private:
    RenderObjClass *m_Proto;
};
