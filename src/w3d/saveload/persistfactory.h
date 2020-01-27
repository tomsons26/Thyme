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
#include "persist.h"
#include "saveloadids.h"

class PersistFactoryClass
{
public:
    PersistFactoryClass();
    virtual ~PersistFactoryClass();
    virtual unsigned int Chunk_ID() const = 0;
    virtual class PersistClass *Load(ChunkLoadClass &cload) const = 0;
    virtual void Save(ChunkSaveClass &csave, PersistClass *obj) const = 0;

public:
    PersistFactoryClass *m_nextFactory;
};
