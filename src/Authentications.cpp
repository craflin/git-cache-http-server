
#include "Authentications.hpp"

#include <nstd/Mutex.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/Time.hpp>

namespace {
    typedef HashMap<String, int64> AuthMap;

    Mutex _mutex;
    AuthMap _authentications;

    String getKey(const String& repo, const String& auth)
    {
        return auth + " " + repo;
    }

    void _cleanup(int64 now)
    {
        while (!_authentications.isEmpty())
        {
            if (now - _authentications.front() < 60 * 60 * 1000) // 1 hour
                break;
            _authentications.removeFront();
        }
    }
}

void storeAuth(const String& repo, const String& auth)
{
    String key = getKey(repo, auth);
    int64 now = Time::time();
    {
        Mutex::Guard guard(_mutex);
        _cleanup(now);
        _authentications.remove(key);
        _authentications.append(key, now);
    }
}

void removeAuth(const String& repo, const String& auth)
{
    String key = getKey(repo, auth);
    {
        Mutex::Guard guard(_mutex);
        _authentications.remove(key);
    }
}

bool checkAuth(const String& repo, const String& auth)
{
    String key = getKey(repo, auth);
    int64 now = Time::time();
    {
        Mutex::Guard guard(_mutex);
        _cleanup(now);
        return _authentications.contains(key);
    }
}
