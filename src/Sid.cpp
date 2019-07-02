#include "Sid.hpp"

// global instance of the debug sid database
// TODO: consider removing this databse in release builds.
SidDatabase* g_debug_sid_database;

void SidDatabase::Initialize(Allocator* allocator)
{
    assert(allocator);
    g_debug_sid_database = allocator->New<SidDatabase>(allocator);
}

void SidDatabase::Terminate()
{
    Allocator* allocator = g_debug_sid_database->allocator;
    g_debug_sid_database->Destroy();
    allocator->Delete(g_debug_sid_database);
}

