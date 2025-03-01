#pragma once

#include <nstd/Mutex.hpp>
#include <nstd/String.hpp>
#include <nstd/PoolMap.hpp>

class NamedMutexGuard
{
public:
    NamedMutexGuard(const String& name);
    ~NamedMutexGuard();

private:
    struct NamedMutex
    {
        Mutex mutex;
        usize count;

        NamedMutex() : count(0) {}
    };

    typedef PoolMap<String, NamedMutex> MutexMap;

private:
    MutexMap::Iterator _object;

private:
    static MutexMap _objects;
};
