/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Classes for storing hashes in tables.
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

class HashableClass
{
public:
    HashableClass() : m_nextHash(0) {}
    virtual ~HashableClass() {}
    virtual const char *Get_Key() = 0;

public:
    HashableClass *m_nextHash;
};

class HashTableClass
{
public:
    HashTableClass(int size);
    ~HashTableClass();
    void Reset();
    void Add(HashableClass *entry);
    bool Remove(HashableClass *entry);
    HashableClass *Find(char const *key);
    int Hash(const char *key);

public:
    int m_hashTableSize;
    HashableClass **m_hashTable;
};

class HashTableIteratorClass
{
public:
    HashTableIteratorClass(HashTableClass &table) : m_table(table) {}
    virtual ~HashTableIteratorClass() {}
    void First();
    void Next();
    bool Is_Done() { return m_currentEntry == nullptr; } // i think, based on enb..
    HashableClass *Get_Current() { return m_currentEntry; }
    HashableClass *Get_Next() { return m_nextEntry; }
    void Advance_Next();

public:
    HashTableClass &m_table;
    int m_index;
    HashableClass *m_currentEntry;
    HashableClass *m_nextEntry;
};
