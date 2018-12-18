#include "hash.h"
#include "crc.h"
#include "gamedebug.h"

HashTableClass::HashTableClass(int size)
{
    m_hashTableSize = size;
    DEBUG_ASSERT((m_hashTableSize & (m_hashTableSize - 1)) == 0);
    m_hashTable = new HashableClass *[m_hashTableSize];
    Reset();
}

HashTableClass::~HashTableClass()
{
    if (m_hashTable) {
        delete (m_hashTable);
    }
    m_hashTable = nullptr;
}

void HashTableClass::Reset()
{
    for (int i = 0; i < m_hashTableSize; ++i) {
        m_hashTable[i] = nullptr;
    }
}

void HashTableClass::Add(HashableClass *entry)
{
    DEBUG_ASSERT(entry != NULL);
    unsigned int hash = Hash(entry->Get_Key());
    entry->m_nextHash = m_hashTable[hash];
    m_hashTable[hash] = entry;
}

bool HashTableClass::Remove(HashableClass *entry)
{
    DEBUG_ASSERT(entry != NULL);
    unsigned int hash = Hash(entry->Get_Key());
    if (!m_hashTable[hash]) {
        return false;
    }
    if (m_hashTable[hash] == entry) {
        m_hashTable[hash] = entry->m_nextHash;
        return true;
    }
    for (HashableClass *i = m_hashTable[hash]; i->m_nextHash; i = i->m_nextHash) {
        if (i->m_nextHash == entry) {
            i->m_nextHash = entry->m_nextHash;
            return true;
        }
    }
    return false;
}

HashableClass *HashTableClass::Find(char const *key)
{
    for (HashableClass *i = m_hashTable[Hash(key)]; i; i = i->m_nextHash) {
        if (!stricmp(i->Get_Key(), key)) {
            return i;
        }
    }
    return nullptr;
}

int HashTableClass::Hash(const char *key)
{
    return (m_hashTableSize - 1) & CRC::Stringi(key, 0);
}

void HashTableIteratorClass::First()
{
    m_index = 0;
    m_nextEntry = m_table.m_hashTable[m_index];
    Advance_Next();
    Next();
}

void HashTableIteratorClass::Next()
{
    m_currentEntry = m_nextEntry;
    if (m_nextEntry) {
        m_nextEntry = m_nextEntry->m_nextHash;
        Advance_Next();
    }
}

void HashTableIteratorClass::Advance_Next()
{
    while (!m_nextEntry) {
        if (++m_index >= m_table.m_hashTableSize) {
            break;
        }
        m_nextEntry = m_table.m_hashTable[m_index];
    }
}
