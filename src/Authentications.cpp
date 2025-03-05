
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
        return repo + " " + auth;
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

bool authRequired(const String& repo)
{
    int64 now = Time::time();
    String key = getKey(repo, String());
    {
        Mutex::Guard guard(_mutex);
        _cleanup(now);
        if (_authentications.contains(key))
            return false;
        for (AuthMap::Iterator i = _authentications.begin(), end = _authentications.end(); i != end; ++i)
            if (i.key().startsWith(key))
                return true;
        return false;
    }
}
