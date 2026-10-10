
#include "UpdateTimes.hpp"

#include <nstd/Mutex.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/Time.hpp>

namespace {
    typedef HashMap<String, int64> UpdateTimeMap;

    Mutex _mutex;
    UpdateTimeMap _updateTimes;
}

void storeUpdateTime(const String& repo, int64 time)
{
    Mutex::Guard guard(_mutex);
    _updateTimes.append(repo, time);
}

int64 getUpdateTime(const String& repo)
{
    {
        Mutex::Guard guard(_mutex);
        UpdateTimeMap::Iterator find = _updateTimes.find(repo);
        if (find != _updateTimes.end())
            return *find;
    }
    return 0;
}
