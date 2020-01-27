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

#include "postloadable.h"

class PersistFactoryClass;
class ChunkSaveClass;
class ChunkLoadClass;

class PersistClass : public PostLoadableClass
{
public:
    virtual const PersistFactoryClass &Get_Factory() const = 0;
    virtual bool Save(ChunkSaveClass &csave) { return true; }
    virtual bool Load(ChunkLoadClass &cload) { return true; }
};
