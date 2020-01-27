#include "persistfactory.h"

/**
 *
 *
 */
PersistFactoryClass::PersistFactoryClass():
m_nextFactory(0)
{
    // i don't think this is of any use for SAGE
    //SaveLoadSystemClass::Register_Persist_Factory(this);
}

/**
 *
 *
 */
PersistFactoryClass::~PersistFactoryClass()
{
    // i don't think this is of any use for SAGE
    //SaveLoadSystemClass::Unregister_Persist_Factory(this);
}
